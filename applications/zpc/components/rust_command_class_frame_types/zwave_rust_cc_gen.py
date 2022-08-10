from os import name
import re
import xml.etree.ElementTree as ET
import argparse


################################# Generic name handling #########################
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

#################################### Struct generation #########################


def generate_marker(parent):
    x = 'pub struct %s {}\n\n' % type_of_param(parent)
    return x


def generate_enum(parent):
    if('value' in parent.attrib):
        return ''
    x = '#[derive(Clone, Copy)]'
    x += 'pub enum %s { \n' % type_of_param(parent)
    for p in parent:
        if(p.tag in ['const', 'fieldenum']):
            x = x + '  %s,\n' % to_upper_camel_case(fix_name(p))
    return x + "}\n\n"


def generate_struct(parent):
    x = 'pub struct %s { \n' % type_of_param(parent)
    for p in parent:
        if(p.tag in ['param', 'bitfield', 'bitflag', 'fieldenum']):
            x = x + "  pub %s : %s, \n" % (fix_name(p), type_of_param(p))
        elif(p.tag in ['variant_group']):
            x = x + "  pub %s : Vec<%s>, \n" % (fix_name(p), type_of_param(p))
    return x + "}\n\n"

####################################### Frame Generator ####################################


def generator_template(from_type, body):
    template = '''
impl From<%s> for FrameParser {
  fn from( _typ: %s ) -> Self {
    let mut frame = FrameParser::new();
%s    frame
  }
}
'''
    return template % (from_type, from_type, body)


def generate_marker_generator(parent):
    maker_name = type_of_param(parent)
    x = ''
    for p in parent:
        if(p.tag in ['const']):
            x = x + \
                'frame.append( FrameParser::from(%s as u8) );\n' % p.attrib['flagmask']
    return generator_template(maker_name, x)


def generate_byte_enum_generator(parent):
    if('value' in parent.attrib):
        return ''

    enum_name = type_of_param(parent)
    n = 0
    x = '''
impl From<%s> for u8 {
  fn from( _typ: %s ) -> Self {
''' % (enum_name, enum_name)
    x = x + '    match _typ {\n'
    for p in parent:
        if(p.tag == 'const'):
            name = to_upper_camel_case(fix_name(p))
            x = x + \
                '      %s::%s => %su8,\n' % (
                    enum_name, name, p.attrib['flagmask'])
        if(p.tag == 'fieldenum'):
            name = to_upper_camel_case(fix_name(p))
            x = x + '      %s::%s => %su8,\n' % (enum_name, name, n)
            n = n + 1

    x = x + '    }\n'
    x = x + '  }\n'
    x = x + '}\n'
    return x


def generate_enum_generator(parent):
    if('value' in parent.attrib):
        return ''

    enum_name = type_of_param(parent)
    x = '    let byte = u8::from( _typ ); \n'
    x = x + '    frame.append( FrameParser::from(byte) );\n'
    return generate_byte_enum_generator(parent) + generator_template(enum_name, x)


def generate_struct_byte_generator(parent):
    struct_name = type_of_param(parent)
    x = '    let mut _byte : u8 = 0;\n'
    for p in parent:
        name = fix_name(p)
        if(p.tag in ['bitfield', 'fieldenum']):
            shift = "0"
            if('shifter' in p.attrib):
                shift = p.attrib['shifter']
            mask = p.attrib['fieldmask']
            x = x + \
                '    _byte = _byte | ((u8::from(_typ.%s) << %s) & %s);\n' % (
                    name, shift, mask)
        if(p.tag == 'bitflag'):
            mask = p.attrib['flagmask']
            x = x + '    if _typ.%s  {_byte = _byte | %s;}\n' % (name, mask)
    x = x + '    frame.append( FrameParser::from(_byte) );\n'
    return generator_template(struct_name, x)


def generate_struct_generator(parent):
    global cur_cc
    struct_name = type_of_param(parent)
    y = ''
    x = ''
    if(parent.tag == 'cmd'):
        x = x + '    frame.append( FrameParser::from(%su8) );\n' % cur_cc
        x = x + \
            '    frame.append( FrameParser::from(%su8) );\n' % parent.attrib['key']

    for p in parent:
        name = fix_name(p)
        if(p.tag in ['param'] and p.attrib['type'] == 'VARIANT'):
            variant = p.findall('variant')[0]
            x = x + '    frame.write_label(%s,%s,_typ.%s.len() as u8);\n' % (
                variant.attrib['paramoffs'], variant.attrib['sizemask'], name)
            x = x + '    for i in _typ.%s {\n' % name
            x = x + '      frame.append( FrameParser::from(i) );\n'
            x = x + '    };\n'
        elif(p.tag in ['param', 'bitfield', 'bitflag']):
            x = x + '    frame.save_write_label(%s) ;\n' % p.attrib['key']
            x = x + '    frame.append( FrameParser::from(_typ.%s) );\n' % name
        elif(p.tag in ['variant_group']):
            x = x + '    frame.write_label(%s,%s,_typ.%s.len() as u8);\n' % (
                p.attrib['sizeoffs'], p.attrib['sizemask'], name)
            x = x + '    for i in _typ.%s {\n' % name
            x = x + '      frame.append( FrameParser::from(i) );\n'
            x = x + '    };\n'

    return y + generator_template(struct_name, x)

#################################### Frame parsing ###################################


def parser_template(from_type, body):
    template = '''
impl TryFrom<&mut FrameParser> for %s {
  type Error = FrameParserError;
  fn try_from(_frame: &mut FrameParser) -> Result<Self, Self::Error> {
      %s
  }
}

'''
    return template % (from_type, body)


def generate_marker_parser(parent):
    maker_name = type_of_param(parent)
    x = ''
    for p in parent:
        if(p.tag in ['const']):
            x = x + \
                'if _frame.read() != %s { return Err(FrameParserError{}) }\n' % p.attrib[
                    'flagmask']
    x = x + 'Ok( %s{} )' % maker_name
    return parser_template(maker_name, x)


def generate_enum_parser(parent):
    if('value' in parent.attrib):
        return ''
    n = 0
    enum_name = type_of_param(parent)
    x = '    let byte = _frame.read();\n'
    x = x + '    match byte {\n'
    for p in parent:
        if(p.tag == 'const'):
            name = to_upper_camel_case(fix_name(p))
            x = x + \
                '      %su8 => Ok(%s::%s),\n' % (
                    p.attrib['flagmask'], enum_name, name)
        elif(p.tag == 'fieldenum'):
            name = to_upper_camel_case(fix_name(p))
            x = x + '      %su8 => Ok(%s::%s),\n' % (n, enum_name, name)
            n = n+1

    x = x + '      _ => Err(FrameParserError{})\n'
    x = x + '    }\n'

    return parser_template(enum_name, x)


def generate_enum_byte_parser(parent):
    if('value' in parent.attrib):
        return ''

    enum_name = type_of_param(parent)
    x = '''impl TryFrom<u8> for %s {
  type Error = FrameParserError;
  fn try_from(byte: u8) -> Result<Self, Self::Error> {
    ''' % enum_name
    x = x + '    match byte {\n'
    i = 0
    for p in parent:
        if(p.tag == 'const'):
            name = to_upper_camel_case(fix_name(p))
            x = x + \
                '      %su8 => Ok(%s::%s),\n' % (
                    p.attrib['flagmask'], enum_name, name)
        elif(p.tag == 'fieldenum'):
            name = to_upper_camel_case(fix_name(p))
            x = x + '      %su8 => Ok(%s::%s),\n' % (i, enum_name, name)
            i = i + 1

    x = x + '      _ => Err(FrameParserError{})\n'
    x = x + '    }\n'
    x = x + '  }\n'
    x = x + '}\n'
    return x


def generate_enum_parser(parent):
    if('value' in parent.attrib):
        return ''

    enum_name = type_of_param(parent)
    x = '    let byte = _frame.read();\n'
    x = x + '    %s::try_from( byte )\n' % enum_name

    return generate_enum_byte_parser(parent) + parser_template(enum_name, x)


def generate_struct_byte_parser(parent):
    struct_name = type_of_param(parent)
    x = '    let byte = _frame.read();\n'
    for p in parent:
        name = fix_name(p)
        if(p.tag in ['bitfield', 'fieldenum']):
            shift = "0"
            if('shifter' in p.attrib):
                shift = p.attrib['shifter']
            mask = p.attrib['fieldmask']
            x = x + '    let %s = (byte & %s) >> %s;\n' % (name, mask, shift)
        if(p.tag == 'bitflag'):
            mask = p.attrib['flagmask']
            x = x + '    let %s = (byte & %s) >0;\n' % (name, mask)

    x = x + '    let result = %s{\n' % struct_name
    for p in parent:
        field_type = type_of_param(p)
        field_name = fix_name(p)
        if(p.tag in ['bitfield', 'bitflag']):
            x = x + '      %s : %s,\n' % (field_name, field_name)
        if(p.tag in ['fieldenum']):
            x = x + \
                '      %s : %s::try_from(%s)?,\n' % (
                    field_name, field_type, field_name)

    x = x + '    };\n'
    x = x + '    Ok(result)'
    return parser_template(struct_name, x)


def generate_struct_byte_conversion(struct_name):
    return '''
  impl From<%s> for Vec<u8> {
    fn from( frame : %s ) -> Self {
      FrameParser::from( frame ).bytes().to_vec()
    }
  }
  ''' % (struct_name, struct_name)


def generate_struct_parser(parent):
    global cur_cc
    struct_name = type_of_param(parent)
    x = ''

    for p in parent:
        name = fix_name(p)
        type_name = type_of_param(p)
        if((type_name[0] == '[') and (type_name[-1] == ']')):  # Arrays needs special treatment
            type_name = '<' + type_name + '>'

        x = x + '    let mut frame2 = _frame.clone();\n'
        if(p.tag == 'param' and p.attrib['type'] == 'VARIANT'):
            variant = p.findall('variant')[0]
            x = x + '    let %s_len = _frame.read_label(%s,%s);\n' % (
                name, variant.attrib['paramoffs'], variant.attrib['sizemask'])
            x = x + '    let mut %s = Vec::new();\n' % name
            x = x + '    for _i in 0..%s_len {\n' % name
            x = x + '      %s.push( u8::try_from(&mut frame2)? );\n' % (name)
            x = x + '    };\n'
        elif(p.tag in ['param', 'bitfield', 'bitflag', 'fieldenum']):
            x = x + '    _frame.save_read_label(%s) ;\n' % p.attrib['key']
            x = x + \
                '    let %s = %s::try_from(&mut frame2)?;\n' % (
                    name, type_name)
        elif(p.tag in ['variant_group']):
            x = x + '    let %s_len = _frame.read_label(%s,%s);\n' % (
                name, p.attrib['sizeoffs'], p.attrib['sizemask'])
            x = x + '    let mut %s = Vec::new();\n' % name
            x = x + '    for _i in 0..%s_len {\n' % name
            x = x + \
                '      %s.push( %s::try_from(&mut frame2)? );\n' % (
                    name, type_name)
            x = x + '    };\n'
        x = x + '    _frame.copy_state(&frame2);\n'

    x = x + '    let result = %s{\n' % struct_name
    for p in parent:
        name = fix_name(p)
        x = x + '      %s,\n' % (name)
    x = x + '    };\n'
    x = x + '    Ok(result)'
    return parser_template(struct_name, x) + generate_struct_byte_conversion(struct_name)

################################### Top command class parser ############################################


def generate_class_handler_handler(cc):
    x = ''
    x = x + 'pub enum %s {\n' % type_of_param(cc)
    for cmd in cc.iter('cmd'):
        x = x + '  %s(%s),\n' % (type_of_param(cmd), type_of_param(cmd))
    x = x + '}\n\n'

    x = x + '''impl TryFrom<&mut FrameParser> for %s {
  type Error = FrameParserError;
  fn try_from(frame: &mut FrameParser ) -> Result<Self, Self::Error> {
''' % (type_of_param(cc))
    x = x + \
        '    if %s != frame.read() { return Err(FrameParserError{}) } ; \n' % (
            cc.attrib['key'])
    x = x + '    match frame.read() { \n'
    for cmd in cc.iter('cmd'):
        x = x + '      %s => Ok(%s::%s(%s::try_from( frame )?)),\n' % (
            cmd.attrib['key'], type_of_param(cc), type_of_param(cmd), type_of_param(cmd))
    x = x + '      _ => Err(FrameParserError{}),\n'
    x = x + '    }\n'
    x = x + '  }\n'
    x = x + '}\n\n'

    x = x + '''
impl TryFrom<&[u8]> for %s {
  type Error = FrameParserError;
  fn try_from(bytes: &[u8] ) -> Result<Self, Self::Error> {
    let mut parser = FrameParser::from( bytes );
    %s::try_from( &mut parser )
  }
}
''' % (type_of_param(cc), type_of_param(cc))

    return x


################################### Main execution ############################################

def foreach_xml(xml, tag, type, func):
    x = ''
    for e in xml.iter(tag):
        if(tag == 'param'):
            if(e.attrib['type'] == type):
                x = x + func(e)
        else:
            x = x + func(e)
    return x


def generate_cmd(cmd):
    global ns
    ns = cmd.attrib['name'].lower() + '_'
    x = ''
    x = x + foreach_xml(cmd, 'fieldenum', '', generate_enum)
    x = x + foreach_xml(cmd, 'param', 'CONST', generate_enum)
    x = x + foreach_xml(cmd, 'param', 'STRUCT_BYTE', generate_struct)
    #x = x + foreach_xml( cmd, 'param','BITMASK', generate_struct)
    x = x + foreach_xml(cmd, 'param', 'MARKER', generate_marker)
    x = x + foreach_xml(cmd, 'variant_group', '', generate_struct)
    x = x + generate_struct(cmd)

    x = x + foreach_xml(cmd, 'fieldenum', '', generate_enum_generator)
    x = x + foreach_xml(cmd, 'param', 'CONST', generate_enum_generator)
    x = x + foreach_xml(cmd, 'param', 'STRUCT_BYTE',
                        generate_struct_byte_generator)
    #x = x + foreach_xml( cmd, 'param','BITMASK', generate_struct_byte_generator)
    x = x + foreach_xml(cmd, 'param', 'MARKER', generate_marker_generator)
    x = x + foreach_xml(cmd, 'variant_group', '', generate_struct_generator)
    x = x + generate_struct_generator(cmd)

    x = x + foreach_xml(cmd, 'fieldenum', '', generate_enum_parser)
    x = x + foreach_xml(cmd, 'param', 'CONST', generate_enum_parser)
    x = x + foreach_xml(cmd, 'param', 'STRUCT_BYTE',
                        generate_struct_byte_parser)
    #x = x + foreach_xml( cmd, 'param','BITMASK', generate_struct_byte_parser)
    x = x + foreach_xml(cmd, 'param', 'MARKER', generate_marker_parser)
    x = x + foreach_xml(cmd, 'variant_group', '', generate_struct_parser)
    x = x + generate_struct_parser(cmd)

    return x


def generate_command_class(cc):
    global cur_cc
    cur_cc = cc.attrib['key']
    x = '''
/// This file is auto generated with rust_command_class_frame_types.py please do not modify
/// Handlers for %s version %s
pub use crate::frameparser::{FrameParser,FrameParserError,Bit24};
use std::convert::TryFrom;

''' % (cc.attrib['name'], cc.attrib['version'])
    x = x + 'pub const %s: u16 = %s;\n' % (cc.attrib['name'], cc.attrib['key'])
    x = x + \
        'pub const %s_VERSION: u8 = %s;\n' % (
            cc.attrib['name'], cc.attrib['version'])

    for cmd in cc.iter('cmd'):
        x = x + generate_cmd(cmd)
    return x + generate_class_handler_handler(cc)


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

    # Collect dict of command classes with highest version number
    cc_dict = {}
    for cc in data_dict.iter('cmd_class'):
        cc_name = fix_name(cc)
        if cc_name in cc_dict:
            if(cc_dict[cc_name].attrib['version'] < cc.attrib['version']):
                cc_dict[cc_name] = cc
        else:
            cc_dict[cc_name] = cc

    # Make handler for each command class

    generated_ccs = []
    for cc_name in cc_dict:
        cc = cc_dict[cc_name]

        try:
            x = generate_command_class(cc)
        except Exception as e:
            print(" 💥 Unable to create handler for %s: %s" % (cc_name, str(e)))
            continue
        print(" ✅ Generated handler for %s" % (cc_name))

        f = open("%s/%s.rs" % (outdir, cc_name), "w")
        f.write(x)
        f.close()
        generated_ccs.append(cc_name)

    # Generate main lib.rs
    lib_rs = ""
    for cc_name in generated_ccs:
        lib_rs += "pub mod %s;\n" % cc_name
        lib_rs += "pub use %s::*;\n" % cc_name

    f = open("%s/lib.rs" % outdir, "w")
    f.write(lib_rs)
    f.close()