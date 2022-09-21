
# UAM maps for the ZPC

## Configuring maps

Additional maps can be added, or the existing default maps can be modified
as needed. Maps are normally used for dotdot/Z-Wave value conversion, but
they can also be used to:

* Fill device data automatically, and skip interviewing known properties
* Configure/Set certain values automatically

### Mapper file structure

A mapper file template could be as follow:

```uam
// this is a comment

def <name1> expression
def <name2> expression

scope 0 {
  <attribute1> = <expression1>
  <attribute2> = <expression2>
}
```

### Mapper grammar and keywords

The mapper allows to manipulate attributes that are stored as 32-bit signed
integer values. Each attribute can use the following "value states":

* `r` for reported value
* `d` for desired value
* `e` for the existence of the attribute. (evaluates to true/false depending on if the attribute exists)

The value states must be used in front of attribute types, using a `'` character.
To perform an assignment, the `=` sign can be used. For example, if the reported
value of attribute type 1 should take the desired value of attribute type 2,
the following expression can be used:

```uam
r'1 = d'2
```

Note: Assignments to reported values will trigger attribute creations
in the attribute store.

* `d'1 = r'2` will not create attribute type 1 and set its desired value
  if it does not exist.
* `r'1 = r'2` will create attribute type 1 and set its reported value
  if it does not exist.

The mapper constantly monitor attribute updates, so whenever
the desired value of attribute 2 is updated, the reported value of attribute
1 will be adjusted.

To avoid using raw type values, the mapper allows to define value substitutions
using the `def` keyword. For example, the example above could be expressed as:

```uam
def zwave_state 1
def zigbee_state 2
r'zwave_state = d'zigbee_state
```

Regular operators can be used to modify the value. The available operators are:

| Operator | Name           |
|----------|----------------|
| +        | Addition       |
| -        | Subtraction    |
| *        | Multiplication |
| /        | Division       |
| &        | bitwise AND    |
| \|       | bitwise OR     |
| %        | Modulo         |
| **       | Exponent       |

For example, if a value needs to be multiplicated by 2 and incremented by 1:

```uam
def zwave_state 1
def zigbee_state 2
r'zwave_state = ((d'zigbee_state * 2) + 1)
```

It is **strongly recommended** to use parentheses around operations, as
no priority handling is implemented in the mapper.

Conditional statements are also available for maps. The available comparison
operators are:

| Operator | Name             |
|----------|------------------|
| ==       | Equal to         |
| !=       | Not equal to     |
| >        | Greater than     |
| <        | Less than        |

These operators should be used with `if` statements. The `if` syntax is as
follows: `if condition value`. A switch-like statement can be made using
multiple `if` statements.

```uam
// If the desired value of attribute 2 is 0, set reported of 1 to 1
// If the reported value of attribute 3 is 3, set reported of 1 to 2
// if none of the previous conditions is true, set the reported of 1 to 0.
r'1 =
  if (d'2 == 0) 1
  if (r'3 == 3) 2
  0
```

Note in the example above that line breaks and indentation does not have any
impact, and are just used for readability. The statement can be written in one
line too. This is an example from the ZPC default maps, which determines if the
reported value of the battery low flag should go to 1 or 0 based on the
reported battery percentage.

```uam
// [...] Skipping def statements
r'zbBATTERY_LOW =
  if (r'zwBATTERY < 10) 1
  if (r'zwBATTERY == 0xFF) 1
  0
```

Note that bitwise operators `&` and `|` can be used for evaluating a set of
boolean expressions:

```uam
r'1 =
  if ((d'2 == 0) & (r'3 == 3)) 12
  if (d'2 == 0) 1
  if (r'3 == 3) 2
  0
```

A special `undefined` keyword can be used to abort assignment evaluations.
For example, to set the reported value of an attribute **only if** another
attribute exists, the following map can be used:

```uam
// If attribute type 2 exists, set the value to 1, else do not do anything
r'1 =
  if (e'2) 1
  undefined
```

Additionally, a special `or` keyword can be used for fallback values:

```uam
// If attribute type 2 has a reported value, set the reported value of 1 to this value
// else set it to the (reported value of attribute type 3) + 2
r'1 = r'2 or (r'3 + 2)
```

```uam
// If attribute type 2 exists, set the value to 1, else do not do anything
r'1 =
  if (e'2) 1
  undefined
```

Tree navigation operators can be used to navigate the attribute store tree,
for when attributes are organized in a more complicated manner.

| Operator | Name                       |
|----------|----------------------------|
| []       | Reported value navigation  |
| .        | Child navigation           |
| \^       | Parent navigation          |

> WARNING:
> Parent navigation operator has some known issues.

```uam
// If the desired value of (attribute 3 placed under attribute 2 with reported value 1) is greater than 0, set reported of 1 to 1
r'1 =
  if (d'2[1].3 > 0) 1
  undefined
```

All attributes are placed under an attribute representing an endpoint.
If trying to navigate up the tree, the parent is the endpoint attribute.

```uam
def ep 0x00000004
// Set the reported value of attribute 1 to the reported value of attribute 1 under endpoint ID 1 to the same value under endpoint 0.
r'^.ep[1].1 = r'^.ep[0].1
```

Finally, all map assignment must have a scope. Multiple scopes for a given map
file are not yet supported, but it will allow in the future to prioritize
set of instructions.

```uam
// Defines
def zwave_state 1
def zigbee_state 2

// Mapping
scope 0 {
r'zwave_state = r'zigbee_state
}
```

### Attribute types

To configure a map, the attribute types that will be mapped must be identifed
first. Each Command Class has a set of defined attribute types that it
allocates in the attribute store under endpoints.

The CLI can be used on a running ZPC to print out the attribute store and
its contents. It is an easy and quick way to find out the list of
attribute types and their values for a given NodeID. For example, showing
the attribute store data for NodeID 2:

```text
ZPC>attribute_store_log 2
(305) NodeID .................................................................. 2
    │───(306) Endpoint ID ..................................................... 0
    │   │───(313) NIF ......................................................... [5e,25,26,33,70,27,32,81,85,59,72,86,7a,73,ef,5a,82]
    │   │───(315) Z-Wave Plus Info Version .................................... 2
    │   │───(316) Binary Switch Command Class version ......................... 1
    │   │───(317) Multilevel Switch Command Class version ..................... 2
    │   │───(318) ZCL Level MinLevel .......................................... 0
    │   │───(319) ZCL Level MaxLevel .......................................... 99
    │   │───(320) Color Switch Command Class version .......................... 1
    │   │───(321) Configuration Command Class version ......................... 1
    │   │───(322) Unknown 0x00002701 .......................................... [01]
    │   │───(323) Unknown 0x00003201 .......................................... [03]
    │   │───(324) Unknown 0x00008101 .......................................... [01]
    │   │───(325) Association Command Class version ........................... 2
    │   │───(326) AGI Command Class version ................................... 1
    │   │───(327) Manufacturer Specific Version ............................... 2
    │   │───(328) Version Version ............................................. 2
    │   │───(329) Firmware Update Version ..................................... 2
    │   │───(330) Powerlevel Version .......................................... 1
    │   │───(337) Binary Switch State ......................................... 0
    │   │   │───(338) Value ................................................... 255
    │   │───(339) Multilevel Switch State ..................................... 0
    │   │   │───(340) Value ................................................... 50
    │   │   │───(341) Duration ................................................ 0
    │   │   │───(342) OnOff (virtual state) ................................... 255
    │   │   │───(343) Substate update ......................................... <undefined> (desired: 0)
    │   │───(344) ZCL OnOff OnOff ............................................. 1
    │   │───(345) Color Switch Color component bitmask ........................ [00,1c]
    │   │───(346) Next Configuration Parameter ID ............................. 0
    │   │───(347) Configuration Bulk Support .................................. 0
```

#### Z-Wave Attributes

Most Z-Wave attributes types are defined in the `attribute_store_defined_attribute_types.h` header file. Most of the time,
attributes are stored directly under the endpoint. In this case, the attribute
can simply be referred to with its type.

For example, the Basic Value reported in the Basic Command Class is defined
here:

```c
///< This represents the current state/value.
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_BASIC_VALUE,
                 ((COMMAND_CLASS_BASIC << 8) | 0x02))
```

The value of `ATTRIBUTE_COMMAND_CLASS_BASIC_VALUE` is 8194 or (0x2002), so it
can be referred to as follow in a `uam` file:

```uam
// Z-Wave Basic value
def zwBasicValue 0x2002
```

Some attributes are defined as part of the auto-generated code. The list
of command classes using auto-generated code can be found in the list of
supported/controlled command classes. If a command class is auto-generated,
there will be a header file in the build folder containing attribute type
definitions. For example, for the Door Lock Command Class, refer to
`zwave_COMMAND_CLASS_DOOR_LOCK_attribute_id.h`

When mapping, we have to be aware of the tree structure. Sometimes,
attributes are placed a few levels under the endpoint.  When this is the case,
command class handlers describe how the tree structure is organized in a
PlantUML diagram included in their header files.

In general, it is recommended to reuse the attribute types defined in the
existing maps, rather than investigate how attribute may be stored in the
attribute tree.

#### DotDot Attributes

Dotdot attributes are automatically determined by their Cluster ID / Attribute ID. They are in most cases placed directly under the endpoint.

The 32 bits values is composed by doing `cluster_id<<16 + attribute_id`.
For example, the OnOff attribute (id = `0`) of the OnOff Cluster (id= `0x0006`) is `0x00060000`.

The Basic Cluster (cluster id = 0) is an exception, to avoid collisions
with Z-Wave attributes. Instead, cluster id `0xba5c` is used for the Basic
Cluster.

If some non-standard attributes are saved (e.g. group ID and Group names for
the group cluster, their definitions is added to
`attribute_store_defined_attribute_types.h`)

## Device fingerprints

A few of the default maps give examples of device fingerprints.
It is recommended to use the Manufacturer ID, Product Type and Product ID,
as a minimum.

```uam
// Substitutions
def zwMANUFACTURER_ID           0x00007202
def zwPRODUCT_TYPE_ID           0x00007203
def zwPRODUCT_ID                0x00007204

// Simple fingerprint
def powerstrip_sample_app ((r'zwMANUFACTURER_ID == 0x00) & (r'zwPRODUCT_TYPE_ID == 0x04) & (r'zwPRODUCT_ID == 0x05))

// Set the Z-Wave Plus Info Version to 10 for PowerStrips
def zw_ZWAVE_PLUS_VERSION 0x00005e02

scope 0 {
r'zw_ZWAVE_PLUS_VERSION =
 if (powerstrip_sample_app) 10
 undefined
}

```

A more complete fingerprint can include the version of a product, as well
as its hardware version. This is equivalent to the fingerprinting of devices
prior to perform OTA firmware updates.

The firmware version is stored as `(major << 16) | (minor << 8) | patch`.

```uam
// Substitutions
def zwMANUFACTURER_ID           0x00007202
def zwPRODUCT_TYPE_ID           0x00007203
def zwPRODUCT_ID                0x00007204
def zwVERSION_REPORT            0x00008602
def zwFIRMWARE_ID               0x00008605
def zwFIRMWARE_VERSION          0x00008606
def zwHARDWARE_VERSION          0x0000860f

// Complete fingerprint
def door_lock_sample_app_v10_17_0_1 ((r'zwMANUFACTURER_ID == 0x00) &  (r'zwPRODUCT_TYPE_ID == 0x04) & (r'zwPRODUCT_ID == 0x01) & (r'zwVERSION_REPORT[1].zwFIRMWARE_ID[0].zwFIRMWARE_VERSION == 0x000A1100) & (r'zwVERSION_REPORT[1].zwHARDWARE_VERSION == 1))

// Set the Z-Wave Plus Info Version to 10 for DoorLock v10.17.0 hw version 1
def zw_ZWAVE_PLUS_VERSION 0x00005e02

scope 0 {
r'zw_ZWAVE_PLUS_VERSION =
 if (door_lock_sample_app_v10_17_0_1) 10
 undefined
}
```

## Mapping between Z-Wave and DotDot

To establish a map that reports data on DotDot, we usually need to set
reported value of the DotDot attribute to match the reported value or the
Z-Wave Attribute.

DotDot attributes, once set, are detected and published automatically on MQTT.
This map will for example publish an OnOff attribute for a Binary Switch
value.

```uam
// Binary switch Command Class
def zwCOMMAND_CLASS_SWITCH_BINARY_VERSION 0x2501
def zwSWITCH_BINARY_STATE 0x2502
def zwSWITCH_BINARY_VALUE 0x2503

// OnOff Cluster
def zbON_OFF_CLUSTER_SWITCH 0x00060000

scope 0 {
// Linking reported attributes zwave -> zigbee
r'zbON_OFF_CLUSTER_SWITCH = r'zwSWITCH_BINARY_STATE.zwSWITCH_BINARY_VALUE
}
```

Given that only the reported is mapped, if an MQTT client tries to set the
DotDot OnOff attribute with a command, nothing will happen on the Z-Wave
side.

To make Z-Wave values updates based on DotDot commands, the desired value
change of a dotdot attribute should be mapped to an update of the desired
Z-Wave value:

```uam
// Linking desired attributes zigbee -> zwave
d'zwSWITCH_BINARY_STATE.zwSWITCH_BINARY_VALUE = d'zbON_OFF_CLUSTER_SWITCH
```

## Automatic lifeline/association establishment

The ZPC can be configured through maps to establish associations. The ZPC
associates itself whenever the AGI group profile of an association group
is set to 0x01 or 0x00.

If the ZPC is the SIS and the AGI profile is set to 0x0001, it will establish
an association forcefully, i.e. it will remove nodes from the association
if this group is already full to get associated itself.

In any other cases (ZPC is not the SIS or group profile is set to 0x0000),
the ZPC will associate itself only if there is capacity left in the
association group.

```uam
def zwMANUFACTURER_ID           0x00007202
def zwPRODUCT_TYPE_ID           0x00007203
def zwPRODUCT_ID                0x00007204

def zwASSOCIATION_GROUP_ID      0x00008503
def zwASSOCIATION_GROUP_PROFILE 0x00005904

// device fingerprint
def powerstrip_sample_app ((r'zwMANUFACTURER_ID == 0x00) & (r'zwPRODUCT_TYPE_ID == 0x04) & (r'zwPRODUCT_ID == 0x05))

// AGI profile value to establish an association or not.
def ESTABLISH_ASSOCIATION 0x0000
def ESTABLISH_LIFELINE    0x0001
def NO_LIFELINE           0xFFFF

scope 0 {
// Establish Lifeline association for group 1:
r'zwASSOCIATION_GROUP_ID[1].zwASSOCIATION_GROUP_PROFILE =
  if (powerstrip_sample_app) ESTABLISH_LIFELINE
  undefined

// Establish a regular association Association Group 2 of the PowerStrip sample application
r'zwASSOCIATION_GROUP_ID[2].zwASSOCIATION_GROUP_PROFILE =
  if (powerstrip_sample_app) ESTABLISH_ASSOCIATION
  undefined
}
```

## Configuring a non-default Wake Up Interval

The ZPC will perform the Wake Up interval configuration as follows:

1. If Wake Up on demand is supported (version 3), then the Wake Up interval will
   be the maximum possible value (or 0 if supported).
2. If the Default Wake Up Interval is advertised, this value will be used
3. Else the closest supported value to the ZPC default configuration default
   Wake Up interval will be used.

UAM files can be used to override this setting for a particular device.
For example:

```uam
// Configure non-default Wake Up intervals for particular nodes.
def zwMANUFACTURER_ID           0x00007202
def zwPRODUCT_TYPE_ID           0x00007203
def zwPRODUCT_ID                0x00007204

def WAKE_UP             0x00008409
def INTERVAL            0x00008402
def CAPABILITIES        0x0000840A
def MINIMUM_INTERVAL    0x00008404
def MAXIMUM_INTERVAL    0x00008405

// Device fingerprints
def sensor_pir_sample_app ((r'zwMANUFACTURER_ID == 0x00) & (r'zwPRODUCT_TYPE_ID == 0x04) & (r'zwPRODUCT_ID == 0x03))

scope 0 {

// SensorPIR runs with aggressive Wake Up Interval for testing purposes.
d'WAKE_UP.INTERVAL =
  if (sensor_pir_sample_app) 20
  undefined
}
```

Values can also be taken from devices capabilities, for example here, setting
the maximum supported value:

```uam
// Configure non-default Wake Up intervals for particular nodes.
def zwMANUFACTURER_ID           0x00007202
def zwPRODUCT_TYPE_ID           0x00007203
def zwPRODUCT_ID                0x00007204

def WAKE_UP             0x00008409
def INTERVAL            0x00008402
def CAPABILITIES        0x0000840A
def MINIMUM_INTERVAL    0x00008404
def MAXIMUM_INTERVAL    0x00008405

// Device fingerprints
def sensor_pir_sample_app ((r'zwMANUFACTURER_ID == 0x00) & (r'zwPRODUCT_TYPE_ID == 0x04) & (r'zwPRODUCT_ID == 0x03))

scope 0 {

// Example of settting automatically to the maximum value:
d'WAKE_UP.INTERVAL =
  if (sensor_pir_sample_app) r'CAPABILITIES.MAXIMUM_INTERVAL
  undefined
}
```
