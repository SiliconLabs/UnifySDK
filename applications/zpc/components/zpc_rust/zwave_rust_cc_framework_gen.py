###############################################################################
# License
# <b>Copyright 2022  Silicon Laboratories Inc. www.silabs.com</b>
###############################################################################
# The licensor of this software is Silicon Laboratories Inc. Your use of this
# software is governed by the terms of Silicon Labs Master Software License
# Agreement (MSLA) available at
## www.silabs.com/about-us/legal/master-software-license-agreement. This
# software is distributed to you in Source Code format and is governed by the
# sections of the MSLA applicable to Source Code.
##
###############################################################################

import os
import re
import xml.etree.ElementTree as ET
import argparse
import glob

################################# Generic name handling #################################
fix_chars = re.compile("[^0-9a-z]")
ns = ''


def to_upper_camel_case(snake_str):
    # Convert snake case to upper camel case
    components = snake_str.split('_')
    return ''.join(x.title() for x in components[0:])


def fix_name(n):
    # Takes an XML node an comes up with a name for the node, the name
    # is always snake case and a valid identifier.
    if(n.tag == 'fieldenum'):
        if('value' in n.attrib):
            attr = n.attrib['value']
        else:
            attr = n.attrib['fieldname']
    elif(n.tag == 'bitfield'):
        attr = n.attrib['fieldname']
    elif(n.tag == 'bitflag'):
        attr = n.attrib['flagname']
    elif(n.tag == 'const'):
        attr = n.attrib['flagname']
    elif(n.tag in ['variant_group', 'param', 'cmd', 'cmd_class']):
        attr = n.attrib['name']
    else:
        attr = "ScriptERROR" + n.tag

    attr = attr.lower()
    attr = fix_chars.sub("_", attr)
    if(attr in ['override', 'type']):  # Rust keywords which needs escaping
        attr = "r#"+attr

    if(attr[0].isdigit()):
        attr = "i" + attr

    # Remove multiple underscores
    attr = re.sub(r"_+", "_", attr)

    return attr


def type_of_param(param):
    tag = param.tag
    global ns
    name = fix_name(param)
    if(name.startswith("r#")):
        name = name[2:]

    if(param.tag == 'bitmask'):  # The bitmask tag does not really hold any type info
        return None

    if(param.tag == 'bitfield'):
        return 'u8'
    elif(param.tag == 'bitflag'):
        return 'bool'
    elif(param.tag == 'param' and param.attrib['type'] == 'BYTE'):
        return 'u8'
    elif(param.tag == 'param' and param.attrib['type'] == 'WORD'):
        return 'u16'
    elif(param.tag == 'param' and param.attrib['type'] == 'BIT_24'):
        return 'Bit24'
    elif(param.tag == 'param' and param.attrib['type'] == 'DWORD'):
        return 'u32'
    elif(param.tag == 'param' and param.attrib['type'] == 'ARRAY'):
        return '[u8; %s]' % param[0].attrib['len']
    elif(param.tag == 'param' and param.attrib['type'] == 'VARIANT'):
        return 'Vec<u8>'
    elif(param.tag == 'param' and param.attrib['type'] == 'STRUCT_BYTE'):
        return to_upper_camel_case(ns+name)+"Bits"
#  elif(param.tag == 'param' and param.attrib['type'] == 'BITMASK'):
#    return to_upper_camel_case(ns+name)+"Bits"
    elif(param.tag == 'param' and param.attrib['type'] == 'CONST'):
        return to_upper_camel_case(ns+name)+"Enum"
    elif(param.tag == 'param' and param.attrib['type'] == 'MARKER'):
        return to_upper_camel_case(ns+name)+"Marker"
    elif(param.tag == 'fieldenum'):
        return to_upper_camel_case(ns+name)+"Enum"
    elif(param.tag == 'variant_group'):
        return to_upper_camel_case(ns+name)+"Vg"
    elif(param.tag == 'cmd'):
        return to_upper_camel_case(name)+"Frame"
    elif(param.tag == 'cmd_class'):
        return to_upper_camel_case(name)+"Frame"

    if(param.tag == 'param'):
        raise Exception('Type param::%s Not implemented ' %
                        param.attrib['type'])
    else:
        raise Exception('Type %s Not implemented ' % param.tag)


################################# Command handler interface generation #################################

def sanitize_source_code(source):
    """_summary_
    Sanatize source code removing unit tests and comments.
    """
    sanitized_source_code_list = []
    for i, line in enumerate(source):
        if "//" in line[:3]:
            continue

        sanitized_source_code_list.append(line)

    return sanitized_source_code_list


def parse_source_files():
    # Restructering the Zwave Command classes prompts for changing this path
    base_path = "./src/rust_command_handlers/zwave_command_classes"
    # base_path = "../applications/zpc/components/zwave_rust/rust_command_handlers/src/zwave_command_classes"
    files = glob.glob(base_path + "/**.rs")
    sanitized_source_code_of_implemented_cc = []
    for file in files:
        with open(f"{file}") as file:
            sanitized_source_code_of_implemented_cc.extend(
                sanitize_source_code(file.readlines()))

    return " ".join(sanitized_source_code_of_implemented_cc)


def generate_command_class_interface_trait(cc, source_code_of_implemented_cc: str):
    includes = "use super::zwave_command_class_interface::FrameControlSupportTrait;\n"
    includes += "use super::zwave_command_class_definitions::{CommandClassVersion, CommandClassType, ZwaveCommandClassInfo};\n"
    includes += "use super::zwave_command_class_definitions::ConnectionInfoTrait;\n"
    includes += "use unify_attribute_store_sys::sl_status_t;\n\n"

    cc_info = generate_command_class_info(cc)

    supported_cc_frames = check_which_frames_for_ccs_is_implemented(
        cc, source_code_of_implemented_cc)

    command_class_logic = generate_cc_frame_handling_implementation(
        cc, supported_cc_frames)

    return includes + "\n" + command_class_logic + "\n\n" + cc_info


def generate_command_class_info(cc):
    info = ""
    info += f"impl ZwaveCommandClassInfo for {type_of_param(cc)} {{\n"
    info += f"\tconst VERSION: CommandClassVersion = {cc.attrib['version']};\n"
    info += f"\tconst COMMAND_CLASS_TYPE_ID: CommandClassType = {cc.attrib['key']};\n"
    info += "}\n"

    return info


def check_which_frames_for_ccs_is_implemented(cc, source_code_of_implemented_cc):
    check_for_frame_implementation = "impl FrameControlSupportTrait for {0}"

    cmds_supported = []
    for cmd in cc.iter('cmd'):
        if not check_for_frame_implementation.format(type_of_param(cmd)) in source_code_of_implemented_cc:
            continue

        cmds_supported.append(cmd)

    return cmds_supported


def generate_cc_frame_handling_implementation(cc, supported_cc_frames):
    number_of_frames_to_handle = len(cc.findall('cmd'))
    any_frames_to_handle = number_of_frames_to_handle > 0
    info_parameter = "_info" if not any_frames_to_handle else "info"

    command_class_trait = ""

    on_support_function = f"\n\t#[allow(unused_variables)]\n\tfn on_support(&self, {info_parameter}: &dyn ConnectionInfoTrait) -> Result<(), sl_status_t> {{"
    on_control_function = f"\n\t#[allow(unused_variables)]\n\tfn on_control(&self, {info_parameter}: &dyn ConnectionInfoTrait) -> Result<(), sl_status_t> {{"
    match_statement = "\t\t\t{0}::{1}(frame) => frame.on_{2}({3}),\n"

    on_support_match_statement = on_support_function
    on_control_match_statement = on_control_function

    if any_frames_to_handle:
        on_support_match_statement += "\n\t\tmatch self { \n"
        on_control_match_statement += "\n\t\tmatch self { \n"

        for cmd in supported_cc_frames:
            on_support_match_statement += match_statement.format(
                type_of_param(cc), type_of_param(cmd), "support", info_parameter)
            on_control_match_statement += match_statement.format(
                type_of_param(cc), type_of_param(
                    cmd), "control", info_parameter
            )

        if len(supported_cc_frames) < number_of_frames_to_handle:
            command_class_trait += "use unify_sl_status_sys::SL_STATUS_NOT_SUPPORTED;\n"
            on_support_match_statement += "\t\t\t_ => Err(SL_STATUS_NOT_SUPPORTED),\n"
            on_control_match_statement += "\t\t\t_ => Err(SL_STATUS_NOT_SUPPORTED),\n"

        on_support_match_statement += "\n\t\t}"
        on_control_match_statement += "\n\t\t}"
    else:
        command_class_trait += "use unify_sl_status_sys::SL_STATUS_NOT_SUPPORTED;\n"
        on_support_match_statement += "\n\t\tErr(SL_STATUS_NOT_SUPPORTED)\n"
        on_control_match_statement += "\n\t\tErr(SL_STATUS_NOT_SUPPORTED)\n"

    on_support_match_statement += "\n\t}\n"
    on_control_match_statement += "\n\t}\n"

    command_class_trait += f"impl FrameControlSupportTrait for {type_of_param(cc)} {{\n"

    command_class_trait += on_support_match_statement
    command_class_trait += on_control_match_statement

    command_class_trait += "\n}"

    return command_class_trait


################################# Detecting if data types was generated #################################


def foreach_xml(xml, tag, type, func):
    x = ''
    for e in xml.iter(tag):
        if(tag == 'param'):
            if(e.attrib['type'] == type):
                x = x + func(e)
        else:
            x = x + func(e)
    return x


def check_it_is_possible_to_generate_struct(parent):
    x = 'pub struct %s { \n' % type_of_param(parent)
    for p in parent:
        if(p.tag in ['param', 'bitfield', 'bitflag', 'fieldenum']):
            x = x + "  pub %s : %s, \n" % (fix_name(p), type_of_param(p))
        elif(p.tag in ['variant_group']):
            x = x + "  pub %s : Vec<%s>, \n" % (fix_name(p), type_of_param(p))

################################# Command handler generator #################################


def generate_command_class_logic(cc, source_code_of_implemented_cc: str, cc_name: str):
    x = '''
/// This file is auto generated with zwave_rust_cc_gen.py please do not modify
/// Handlers for %s version %s
use rust_command_class_frame_types::%s::*;
''' % (cc.attrib['name'], cc.attrib['version'], cc_name)

    # Do not create command handlers for types which can't be generated.
    for cmd in cc.iter('cmd'):
        check_it_is_possible_to_generate_struct(cmd)

    return x + generate_command_class_interface_trait(cc, source_code_of_implemented_cc)


def collect_dictionary_of_cc(data_dict):
    cc_dict = {}
    for cc in data_dict.iter('cmd_class'):
        cc_name = fix_name(cc)
        if cc_name in cc_dict:
            if(cc_dict[cc_name].attrib['version'] < cc.attrib['version']):
                cc_dict[cc_name] = cc
        else:
            cc_dict[cc_name] = cc

    return cc_dict


def generate_command_class_files(cc_dict, outdir, source_code_of_implemented_cc):
    generated_ccs = []
    for cc_name in cc_dict:
        cc = cc_dict[cc_name]

        try:
            if "grp_info" in cc_name or "central_scene" in cc_name:
                continue

            x = generate_command_class_logic(
                cc, source_code_of_implemented_cc, cc_name)
        except Exception as e:
            print(" 💥 Unable to create handler for %s: %s" % (cc_name, str(e)))
            continue
        print(" ✅ Generated handler for %s" % (cc_name))

        f = open("%s/%s_impl.rs" % (outdir, cc_name), "w")
        f.write(x)
        f.close()
        generated_ccs.append(cc_name)

    return generated_ccs


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Make command class parsers for rust.')
    parser.add_argument('--xml', required=True,
                        help='Z-Wave command class xml file')
    parser.add_argument('--outdir', required=True,
                        help='directory to write command pasers to')
    args = parser.parse_args()

    xmlfile = args.xml
    outdir = args.outdir

    #xmlfile = 'switch_color.xml'
    with open(xmlfile, 'r', encoding='utf-8') as x_file:
        data_dict = ET.fromstring(x_file.read())
        x_file.close()

    cc_dict = collect_dictionary_of_cc(data_dict)

    source_code = parse_source_files()

    generated_ccs = generate_command_class_files(cc_dict, outdir, source_code)
    # Generate main lib.rs
    lib_rs = ""
    for cc_name in generated_ccs:
        lib_rs = lib_rs + "mod %s_impl;\n" % cc_name

    f = open("%s/lib.rs" % outdir, "w")
    f.write(lib_rs)
    f.close()
