# Unify Debugging Guide

## ZPC Debugging Guide

### Interpreting ZPC Console Logs for Various Operations

#### Using the ZPC Command Line Interface

Pressing return in ZPC console logs will open Unify Command Line Interface.
"help" command will display the available commands in Command Line Interface as
shown below.

```bash
ZPC>help
==================================================
Unify Command line interface Help:
==================================================
[...]
ZPC>
```

Several types of commands will be available. They use similar prefixes
and suffixes. For example,
* __attribute_store___ provides access to Attribute Store functionalities
* __zwave___ provides access to Z-Wave functionalities
* ___log__ prints out information about the state of a component

Note that the Command Line commands should only be used to log information and
are not recommended to trigger changes, such as network management operations or
modifying data in the Attribute Store.

When an unexpected outcome occurs, log the state of the components that are
expected to perform an action.

#### Incoming Z-Wave Frame to ZPC from Z-Wave Module

Incoming Z-Wave API frames are printed directly to the console, which allows
detecting incoming Z-Wave frames or other messages from the Z-Wave API module.

For example,

```bash
[zwapi_protocol_rx_dispatch] Incoming Z-Wave API frame (hex): Length=12 Type=00 Cmd=A8 00 00 01 00 02 04 86 14 5E 02 00 C0 00 7F 7F
```

If the ZPC is communicating to the nodes with S2/S0, the log message will show
encrypted frames. The exact frame payload is typically not printed out.

Shortly after receiving the frame, the Command Class handler will print out the
outcome of processing the Z-Wave frame:

```bash
[zwave_command_handler_callbacks] Command Class=0x25 - Command=0x03 from NodeID 004:1 handled successfully.
```

#### Outgoing Z-Wave Frame from ZPC to Z-Wave Module

Outgoing Z-Wave frames can be identified in the console log. For example,

```bash
[zwave_tx_queue] Enqueuing new frame (id=0x24) - 001:0 ->  002:0 - Encapsulation 1 - Payload [6C 01 A4 04 8E 01 01 01 ]
```

A single frame can result in several TX Queue elements, so multiple TX Queue
messages may appear.

#### Z-Wave Module Information

The following console log describes various things about Z-Wave module.

```bash
[zwave_rx] Z-Wave API module type: 7
[zwave_rx] Z-Wave API module revision: 0
[zwave_rx] Z-Wave API module software version: 7.15
[zwave_rx] Z-Wave API module Manufacturer ID 0x0000
[zwave_rx] Z-Wave API module Product type 0x0004
[zwave_rx] Z-Wave API module Product ID 0x0004
[zwave_rx] Z-Wave API module supported setup mask 0xFE
[zwave_rx] Z-Wave API module provides a Controller API
[zwave_rx] Z-Wave API module is primary or inclusion controller
[zwave_rx] Z-Wave API module is SIS in the network
[zwave_rx] Z-Wave API module library type: 7
[zwave_rx] Z-Wave API module RF region 1
```

#### Z-Wave Protocol Version

The following console log describes version information about Z-Wave Protocol.
```bash
[zwave_rx] Z-Wave API protocol type: 0
[zwave_rx] Z-Wave API protocol version: 7.15.03
[zwave_rx] Z-Wave API protocol build number: 211
[zwave_rx] Z-Wave API protocol git commit: 30313233343536373839414243444546
```

#### S2 Keys

S2 keys are printed in console logs under each key level header.

S2 Unauthenticated:     Key class 1
S2 Authenticated:       Key class 2
S2 Access:              Key class 4
S2 Authenticated LR:    Key class 8
S2 Access LR:           Key class 10

For example,

```bash
[zwave_s2_keystore] Key class 1
[zwave_s2_keystore] FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
[zwave_s2_keystore] Key class 2
[zwave_s2_keystore] CFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
```

#### Unify Build Version

Unify Build version is printed on the first line after the ZPC starts. It looks
like the following:

```bash
# uic build: ver_0.0.6_RC1-339-g3af36c4c
```

#### MQTT Subscriptions Done by ZPC

```bash
[mqtt_client] Subscribing to: ucl/by-group/+/Basic/Commands/WriteAttributes
[mqtt_client] Subscribing to: ucl/by-group/+/Basic/Commands/ResetToFactoryDefaults
[mqtt_client] Subscribing to: ucl/by-group/+/PowerConfiguration/Commands/WriteAttributes
[mqtt_client] Subscribing to: ucl/by-group/+/DeviceTemperatureConfiguration/Commands/WriteAttributes
```

### Changing Log Debug Level in ZPC

#### Log Severity

Log level of ZPC can be changed by using the following command line argument or config
file option.

```bash
zpc  --log.level arg (=d)
```

d: Debug
i: Info
w: Warning
e: Error
c: Critical

#### Component-Based Log Severity

Log levels of ZPC can also be changed on component granularity by using the
following command line argument or config file option for each component.

Tag based log level

```bash
# Format: <component>:<severity>,<component>:<severity>
zpc   --log.tag_level component1:level,component2:level
```

For example, to set ZPC MQTT log level to Info(i) and UIC Main log level to
Error(e), use the following command line.

```bash
zpc --log.tag_level uic_mqtt:i,uic_main:e
```


### Collecting Z-Wave Serial Logs in ZPC

Serial Log collection in ZPC can be enabled by using the following command line
argument or config file option.

```bash
zpc --zpc.serial_log_file <filename>
```

If this is set, the ZPC will write a log of the communication over the serial
interface with the Z-Wave module to the path provided. If the file exists, the
log will be appended to this file, otherwise, the file will be created. The
ZPC will not handle log rotation.

`scripts/serial_decode_zpc.py` can be used to parse this serial log file to better
human readble format

Usage:

```console
python serial_decode_zpc.py serial_log.txt
```
### Using MQTT Explorer to Monitor Messages

You can publish MQTT message and observe the MQTT messages stored in the broker
with a [MQTT Explorer](https://mqtt-explorer.com/).
[MQTT Explorer](https://mqtt-explorer.com/) is an MQTT client that provides a
structured overview of your MQTT topics.

Configure [MQTT Explorer](https://mqtt-explorer.com/) to connect to the host
where your MQTT broker is located and the topics can be observed. Use the
[Unify Specifications](./unify_specifications/index.rst)
if you need assistance to understand what topics and they data represent.
