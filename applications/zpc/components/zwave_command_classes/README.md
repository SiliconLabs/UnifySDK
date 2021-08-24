# generator.py
Application for Z-Wave command class handlers generation.
 
Application generates z-wave frame parsers/builders, extracts attributes from frame,

stores attributes into attribute storage.

Install python requirements

``` bash
pip install -r requirements.txt
```
Run ` python3 generator.py -h ` for usage/help on the generator script

Run attribute header file generation example:

``` bash
python3 generator.py \
  -x ../assets/ZWave_custom_cmd_classes.xml \
  -t ../templates/zwave_{{_name}}_attribute_id.h \
  -t ../templates/zwave_{{_name}}_attribute_id.uam \
  -t ../templates/zwave_{{_name}}_handlers.cpp \
  -t ../templates/zwave_{{_name}}_handlers.h \
  -m ../assets/modificators.json -o generated/src
```
Template files location:
```
templates/
```
Template file names:
```
zwave_{{_name}}_handlers.cpp - for command class handler source file generation
zwave_{{_name}}_handlers.h - for command class handler header file generation
zwave_{{_name}}_attribute_id.h - for command class attribute header file generation
zwave_{{_name}}_attribute_id.uam - for command class attribute mapper file generation
```

Generated header file name:
```
zwave_<command_class_name>_handlers.h - for command class handler header file
zwave_<command_class_name>_attribute_id.h - for command class attribute header file
```

Generated source file name:
```
zwave_<command_class_name>_handlers.cpp - for command class handler source file
```

Generated source file name:
```
zwave_<command_class_name>_attribute_id.uam - for command class handler mapper file
```

Modificators file example:
```json
{
    "version": 1,
    "name": "Z-Wave command class model modificators",
    "attr_aliases_dict": {
      "SWITCH_BINARY_TARGET_VALUE": "SWITCH_BINARY_VALUE",
      "MULTI_CHANNEL_END_POINT_FIND_END_POINT": "MULTI_CHANNEL_END_POINT",
      "MULTI_CHANNEL_CAPABILITY_END_POINT": "MULTI_CHANNEL_END_POINT"  
    },
    "cc_exclusion_list": [
      "COMMAND_CLASS_SUPERVISON",
      "COMMAND_CLASS_SECURITY",
      "COMMAND_CLASS_SECURITY_2",
      "COMMAND_CLASS_NETWORK_MANAGEMENT_INCLUSION",
      "COMMAND_CLASS_FIRMWARE_UPDATE_MD"
    ],

    "cc_exclusion_list": [
    {"cc_name": "COMMAND_CLASS_FIRMWARE_UPDATE_MD"          ,  "min_cc_version": "0", "max_cc_version": "0"},
    {"cc_name": "COMMAND_CLASS_MULTI_INSTANCE"              ,  "min_cc_version": "0", "max_cc_version": "0"},      
    {"cc_name": "COMMAND_CLASS_MULTI_CHANNEL"               ,  "min_cc_version": "3", "max_cc_version": "255"},
    ],
    "attr_child_parent_dict": {
      "THERMOSTAT_SETPOINT_CAPABILITIES_SCALE_": "THERMOSTAT_SETPOINT_SETPOINT_TYPE",
      "THERMOSTAT_SETPOINT_CAPABILITIES_MAXVALUE": "THERMOSTAT_SETPOINT_SETPOINT_TYPE",
      "THERMOSTAT_SETPOINT_CAPABILITIES_MIN_VALUE": "THERMOSTAT_SETPOINT_SETPOINT_TYPE",
      "THERMOSTAT_SETPOINT_CAPABILITIES_PRECISION_": "THERMOSTAT_SETPOINT_SETPOINT_TYPE"
    }
}
```
`attr_aliases_dict` - aliases dictionary for attributes name (to change the name of attribute)

`cc_exclusion_list` - name changing exclusion list (to exclude from passing name changing 

algorithm for command classes that changed name and changed a lot from some version - command classes with 

name == cc_name and version < cc_version will be excluded)

`attr_child_parent_dict` - attributes child-parent dictionary (to change attribute parent)


Python packages list:
```
pybars3==0.9.7
PyMeta3==0.5.1
```
Package licenses (generated with pip-licenses):
```
Name           Version  License     
PyMeta3        0.5.1    MIT License 
pybars3        0.9.7    UNKNOWN     
```