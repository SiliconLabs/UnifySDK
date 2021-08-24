"""@package generator.py
Application for Z-Wave command class handlers generation.

Application generates z-wave frame parsers/builders, extracts attributes from frame,
stores attributes into attribute storage.
"""
import argparse
import collections
import json
import ntpath
import os
import pprint
import re
import xml.etree.ElementTree as ET
from xml.dom import minidom

from pybars import Compiler

# Globals
fix_chars = re.compile("[^A-Z]+[^A-Z0-9]*")
path = os.path.dirname(os.path.abspath(__file__))

type_length = {
    'BIT': 0,
    'BYTE': 1,
    'CONST': 1,
    'STRUCT_BYTE': 1,
    'BITMASK': 1,
    'WORD': 2,
    'BIT_24': 3,
    'DWORD': 4,
    'MARKER': 2,
    'VARIANT': -1,
}

is_report = False
is_get = False
is_set = False
parent = None
cc = ''

command_classes = []
attr_aliases = dict()
param_version = dict()
cc_versions = dict()
cc_attributes = dict()

# Functions


def xml2dict(node):
    d = {}
    for a in node.attrib:
        d['_'+a] = node.attrib[a]

    for child in node:
        if(not child.tag in d):
            d[child.tag] = []
        d[child.tag].append(xml2dict(child))
    return d


def param_fixup(attr_obj, e):
    att = attr_obj[e['_attribute']] if '_attribute' in e else None
    e['att'] = att
    e['has_attr'] = True if att else False
    e['_shifter'] = e['_shifter'] if '_shifter' in e else "0"
    e['_fieldmask'] = e['_fieldmask'] if '_fieldmask' in e else "0xff"
    return e


def command_base_name(command_name):
    if(command_name[-4:] == "_GET"):
        return command_name[:-4]
    elif(command_name[-4:] == "_SET"):
        return command_name[:-4]
    elif(command_name[-7:] == "_REPORT"):
        return command_name[:-7]
    else:
        return None


def attribute_name(cmd_name, param_name):
    global attr_aliases

    attr = command_base_name(cmd_name)+"_"+param_name
    attr = attr.upper()
    attr = fix_chars.sub("_", attr)

    if('RESERVED' in attr):
        return None
    elif('SIZE' in attr):
        return None
    elif(attr in attr_aliases):
        return attr_aliases[attr]
    return attr


def add_attribute(cmd, param, version):
    global parent
    global is_report
    global is_get
    global is_set
    global cc

    attr = attribute_name(cmd.attrib['name'], param.attrib['name'])
    if(not attr):
        return
    param.attrib['attribute'] = attr

    if(not attr in cc_attributes[cc]):
        cc_attributes[cc][attr] = {"name": attr,
                                   "version": version, "parameter": False}
    if(is_get):
        cc_attributes[cc][attr]['parameter'] = True
    if(is_set):
        cc_attributes[cc][attr]['set_func'] = cmd.attrib['name']
    if(is_report):
        cc_attributes[cc][attr]['parent'] = parent
        if(cc_attributes[cc][attr]['parameter']):
            parent = attr
        cc_attributes[cc][attr]['report_func'] = cmd.attrib['name']
        cc_attributes[cc][attr]['get_func'] = command_base_name(
            cmd.attrib['name'])+"_GET"
    return attr


# Helper functions for template engine
def _cmd_type(this, options, suffix):
    result = []
    if(not 'cmd' in this.context):
        return []
    for c in this['cmd']:
        if c['_name'].endswith(suffix):
            c['_cc_key'] = this['_key']
            result.extend(options['fn'](c))
    return result


def _param(this, options):
    result = []
    attr_obj = cc_attributes[this['_cc_key']]
    if(not 'param' in this.context):
        return result

    for param in this['param']:
        # It removes STRUCTE_BYTE as attribute (only bitfields left)
        if(param['_type'] == "STRUCT_BYTE"):
            # Identify bitfield's length
            bitfields_len = len(param['bitfield'])-1
            for e1 in param['bitfield']:
                e = param_fixup(attr_obj, e1)
                e['_length'] = 0
                # Identify last bitfield
                if param['bitfield'].index(e1) == bitfields_len:
                    e['_last_bitfield'] = 1
                result.extend(options['fn'](e))

        else:
            e = param_fixup(attr_obj, param)

            if(param['_type'] in type_length):
                param['_length'] = str(type_length[param['_type']])
            if(param['_type'] == "ARRAY"):
                param['_length'] = param['arrayattrib'][0]['_len']

            if(param['_type'] == "VARIANT"):
                v = e['variant'][0]
                e['_size_key'] = v['_paramoffs']
                e['_size_mask'] = v['_sizemask']

            result.extend(options['fn'](e))

    return result


def _attr_id(this):
    attr_id = int(this["_key"], 16) | (int(this.parent["_key"], 16) << 8)
    return hex(attr_id)

def _human_readable_cc(this, text):
    return text.replace("Command Class ", "")

def equals(this, val1, val2):
    return val1 == val2


def greater(this, val1, val2):
    return int(val1) > int(val2)


def _attr_children(this, options, attr):
    result = []
    for c in this.parent['attribute']:
        if('_parent' in c and c['_parent'] == this['_name']):
            result.extend(options['fn'](c))
    return result


def main():
    global attr_aliases
    global is_get
    global is_set
    global is_report
    global parent
    global cc

    parser = argparse.ArgumentParser()
    parser.add_argument('-x', type=str, required=True, help="defines XML file")
    parser.add_argument('-t', type=str, required=True,
                        help="defines template file", action='append')
    parser.add_argument('-o', type=str, required=True,
                        help="output directory to save generated files")
    parser.add_argument('-m', type=str, required=True,
                        help="defines model modificator file")
    parser.add_argument('-d', action='store_true', required=False,
                        help="Dry run, don't write to output files")

    # Parse Command Line arguments
    args = parser.parse_args()

    xml_file = args.x
    output_dir = args.o
    dry_run = args.d

    # Check if output folder exists and if dry_run mode activated
    if not dry_run and not os.path.exists(output_dir):
        os.makedirs(output_dir)

    with open(xml_file, 'r', encoding='utf-8') as x_file:
        data_dict = ET.fromstring(x_file.read())
        x_file.close()

    child_parent_dict = {}
    cc_exclusion_list = []

    # Check if modificators file exists
    if args.m:
        mod_file = args.m

        with open(mod_file, 'r', encoding='utf-8') as m_file:
            m_data = json.load(m_file)
            m_file.close()

        if 'attr_aliases_dict' in m_data:
            attr_aliases = m_data['attr_aliases_dict']

        if 'attr_child_parent_dict' in m_data:
            child_parent_dict = m_data['attr_child_parent_dict']

        if 'cc_exclusion_list' in m_data:
            cc_exclusion_list = m_data['cc_exclusion_list']


    for cmd_class in data_dict.findall('cmd_class'):
        version = cmd_class.attrib['version']
        cc_name = cmd_class.attrib['name']
        for excl_name in cc_exclusion_list:
            if(cc_name == excl_name['cc_name'] and (
                (int(version) < int(excl_name['min_cc_version'])) or 
                (int(version) > int(excl_name['max_cc_version']))) ):
                try:
                    data_dict.remove(cmd_class)
                except ValueError:
                    pass

    # Make sure that everything is ordered by version
    data_dict[:] = sorted(
        data_dict, key=lambda child: child.attrib['key'] + child.attrib.get('version', ''))

    # Fix chaning names
    cc_names = dict()
    cc_cmd_names = dict()
    for cmd_class in data_dict.iter('cmd_class'):
        cc_names[cmd_class.attrib['key']] = cmd_class.attrib['name']
        for cmd in cmd_class.iter('cmd'):
            cc_cmd_names[cmd_class.attrib['key'] +
                         cmd.attrib['key']] = cmd.attrib['name']

    for cmd_class in data_dict.iter('cmd_class'):
        cc_names[cmd_class.attrib['key']] = cc_names[cmd_class.attrib['key']]
        for cmd in cmd_class.iter('cmd'):
            cmd.attrib['name'] = cc_cmd_names[cmd_class.attrib['key'] +
                                              cmd.attrib['key']]

    # Do the work
    for cmd_class in data_dict.iter('cmd_class'):
        version = cmd_class.attrib['version']
        cc = cmd_class.attrib['key']
        cc_name = cmd_class.attrib['name']
        cc_versions[cc] = version

        if(not cc in cc_attributes):
            cc_attributes[cc] = collections.OrderedDict()

        # sort the command such that the REPORTS comes after the set
        for cmd in sorted(cmd_class.iter('cmd'), key=lambda x: x.attrib['name'][-4:], reverse=True):
            # Setup implicit state variables used by add attribute
            is_get = cmd.attrib['name'].endswith("_GET")
            is_set = cmd.attrib['name'].endswith("_SET")
            is_report = cmd.attrib['name'].endswith("_REPORT")
            parent = None

            for param in cmd.iter('param'):
                # build a hash of the command class and the paremeter, we do this because
                # sometimmes the parameters change name from one cc version to the next
                id = cc + cmd.attrib['key'] + param.attrib['key']
                if(not id in param_version):
                    param_version[id] = version
                param.attrib['version'] = param_version[id]

                if(command_base_name(cmd.attrib['name'])):
                    # assign attributes to fields in struct bytes
                    if(param.attrib['type'] in ["STRUCT_BYTE"]):
                        for e in param:
                            # treat field enum and bitfields identical
                            if(e.tag == 'fieldenum' or e.tag == 'bitfield'):
                                e.tag = 'bitfield'
                                e.attrib['name'] = e.attrib['fieldname']
                                del(e.attrib['fieldname'])
                                add_attribute(cmd, e, version)
                    else:
                        add_attribute(cmd, param, version)

        # Add the attribute elements to the XML
        # The version element is alway there with id 1
        ET.SubElement(cmd_class, 'attribute', name=(
            cc_names[cc]+"_VERSION"), key="1", version="0").text = '\n'
        n = 2
        for a in cc_attributes[cmd_class.attrib['key']].values():
            e = ET.SubElement(cmd_class, 'attribute')
            e.attrib = a
            e.attrib['key'] = str(hex(n))
            e.text = '\n'
            n = n+1

    # Remove older versions, note this works because we have sorted the XML file by version
    # version order in the xml file
    for cmd_class in data_dict.findall('./cmd_class'):
        version = cmd_class.attrib['version']
        expected_version = cc_versions[cmd_class.attrib['key']]
        if(expected_version != version):
            #print("Removing %s version %s != %s" % (cmd_class.attrib['name'], version,expected_version))
            data_dict.remove(cmd_class)

    compiler = Compiler()
    helpers = {
        'cmd_type': _cmd_type,
        'param': _param,
        'equals': equals,
        'greater': greater,
        'attr_children': _attr_children,
        'attr_id': _attr_id,
        'human_readable_cc': _human_readable_cc,
    }

    dd = xml2dict(data_dict)

    for template_file in args.t:
        hbs = open(template_file)
        template = compiler.compile(hbs.read())

        file_template = compiler.compile(ntpath.basename(template_file))

        for cmd_class in dd['cmd_class']:
            if child_parent_dict.keys():
                if 'attribute' in cmd_class:
                    for attr in cmd_class['attribute']:
                        if attr['_name'] in child_parent_dict.keys():
                            # print(attr['_name'])
                            # print("Old parent: {}".format(attr['_parent']))
                            attr['_parent'] = child_parent_dict[attr['_name']]
                            # print("New parent: {}".format(attr['_parent']))

            outfile = file_template(cmd_class)

            if dry_run:
                print(output_dir + "/" + outfile)
            else:
                f = open(output_dir + "/" + outfile, "w")
                f.write(template(cmd_class, helpers=helpers))
                f.close()


if __name__ == "__main__":
    main()
