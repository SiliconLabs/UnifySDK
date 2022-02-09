# Unify SDK Overview

Latest released version of the Unify SDK Overview is available online
[here](https://siliconlabs.github.io/UnifySDK/userguide/html/index.html).

## Introduction

The Unify SDK provides software source code and binary packages for Raspberry Pi 4 to
help build an IoT gateway product. The Unify SDK enables an IoT service to
control and manage end nodes in several wireless PHY radios (e.g., Z-Wave,
Zigbee, and so on) supported by Silicon Labs. The Unify SDK uses the Unify
Controller Language (UCL) as an internal abstraction layer to seamlessly control
various end nodes that are enabled with multiple wireless PHY radios. The
detailed architecture of the Unify SDK is discussed in the
[Unify SDK Specification](https://siliconlabs.github.io/UnifySDK/unify-spec/html/index.html).

The Unify SDK uses Unify SDK library to decouple API modules from the PHY
drivers. This facilitates code reuse and enables easily adding new high-level
APIs without the need to modify the PHY drivers. The library provides a number
of components that simplify the task of writing a PHY driver also known as a
Protocol Controller.

## Systems Overview

A Unify gateway consists of a Message Queuing Telemetry Transport (MQTT) broker
and a number of MQTT clients. The Unify SDK uses the Mosquitto MQTT broker.

The Unify MQTT clients can be grouped into several applications:

- Z-Wave Protocol Controller (ZPC)
- ZigBee Protocol Controller (ZigPC)
- Unify SDK SmartStart Provisioning list (UPVL)
- Unify SDK Group Management Service (GMS)
- Unify Name and Location Service (NAL)
- Unify SDK OTA Image Provider (Image Provider)
- IoT Services (e.g. Developer GUI, UPTICap)

![SDK Overview](doc/assets/img/SDKOverview.png)

The Unify SDK was previously known as Unified IoT Controller or UIC for short.
The old name can still be found in some parts of the SDK.

### Core Unify Components

The Unify SDK contains the following core components:

#### Protocol Controllers

The Unify SDK currently includes the following protocol controllers:

- Z-Wave Protocol Controller ([ZPC](applications/zpc/readme_user.md))
- ZigBee Protocol Controller ([ZigPC](applications/zigpc/readme_user.md))
- Angle of Arrival/Departure Protocol Controller ([AoXPC](applications/aox/applications/aoxpc/readme_user.md))

Each protocol controller interfaces with its radio hardware and implements a
translation between its own wireless protocol and the _Unified Command Language_
(UCL), which is defined in the [Unify SDK Specification](https://siliconlabs.github.io/UnifySDK/unify-spec/html/index.html).
In addition, these protocol controllers implement best practices regarding
message delivery, and adherence to regulatory requirements. Protocol-specific
implementation details can be found in the user guide for each protocol
controller.

#### The Unified SDK Provisioning List (UPVL)

[The UPVL](applications/upvl/readme_user.md) serves the SmartStart Provisioning
list to perform SmartStart Security 2 (S2) inclusions and maintains the
ucl/SmartStart MQTT topic.

#### The Unify SDK Group Manager (GMS)

[GMS](applications/gms/readme_user.md) manages groups and bookkeeping between
protocol controllers, and also publishes group state changes to the ucl/by-group
MQTT topic.

#### The Unify Name and Location service (NAL)

[NAL](applications/nal/readme_user.md) is a helper MQTT component that allows to perform book-keeping on text names and locations that have been assigned. This functionality allows IoT Services to assign and readback a Name and a Location for each node/endpoint.

#### The Unify SDK OTA Image Provider

[OTA Image Provider](applications/image_provider/readme_user.md) announces OTA
images available in OTA storage and publishes OTA binary on request.

#### The Unify AoX Positioning Application

[AoX Positioning application](applications/aox/applications/positioning/readme_user.md)
reads configuration and data from AoXPCs and publish the calculated position
of asset tags in the system.

### IoT Services

#### The Developer UI

[Developer GUI (dev_gui)](applications/dev_ui/dev_gui/readme_user.md)

The dev_gui service is a graphical user interface provided for operating and
provisioning IoT devices using UCL. The dev_gui can be used as a reference for
basic Unify concepts, such as device provisioning and control. The user
interface is provided strictly for test and demonstration purposes only and is
not suitable for production.

#### The UPTICap

[UPTICap (upti_cap)](applications/upti_cap/readme_user.md) is an application to communicate with Silicon Labs WSTK adapters,
to capture data from adapters debug channel and to publish captured data as MQTT messages. Application provided strictly for
test and demonstration purposes only and is not suitable for production.

## Applications Overview

The Unify SDK consists of several components including Protocol Controllers, IoT
Services (e.g. Developer GUI, UPTICap) and multiple application services that
facilitate various functionalities such as SmartStart Provisioning, Group
Management, Name and Location Service and OTA Image Provider.


Please read the [Unify User Guide](doc/readme_user.md) on details on general running
and configuration of the Unify components.

Please read the [Unify Build Guide](doc/readme_building.md) on details on building the unify
SDK.

The following table presents the relation between generic unify applications to
the Unify Protocol Controller. If a given application service is supported via a
Protocol Controller, it will be marked as [x].

|                                                                             | [ZPC](applications/zpc/readme_user.md)   | [ZigPC](applications/zigpc/readme_user.md)  | [AoXPC](applications/aox/applications/aoxpc/readme_user.md)   |
|-----------------------------------------------------------------------------|:-----------------------------------------|:--------------------------------------------|:--------------------------------------------------------------|
| [UPVL](applications/upvl/readme_user.md)                                    | [x]                                      | [x]                                         |                                                               |
| [GMS](applications/gms/readme_user.md)                                      | [x]                                      | [x]                                         |                                                               |
| [NAL](applications/nal/readme_user.md)                                      | [x]                                      | [x]                                         | [x]                                                           |
| [OTA Image Provider](applications/image_provider/readme_user.md)            | [x]                                      | [x]                                         |                                                               |
| [Dev GUI](applications/dev_ui/dev_gui/readme_user.md)                       | [x]                                      | [x]                                         | [x]                                                           |
| [UPTICap](applications/upti_cap/readme_user.md)                             | [x]                                      | [x]                                         |                                                               |
| [AoX Positioning ](applications/aox/applications/positioning/readme_user.md)|                                          |                                             | [x]                                                           |


### Communication Overview

All individual Unify components communicate via MQTT. In this reference
implementation, the Mosquitto MQTT broker is used which does not support
clustering. As a result, all nodes connect to a single central broker.

MQTT implements a publisher-subscriber model, where all payloads are published
to topics to which zero, one, or many clients can subscribe to. By default,
publishers can publish to any topic and all subscribers can subscribe to any
topic. MQTT has the notion of access-control but that functionality is currently
not used by Unify.

A publisher does not get notified (by the broker) if a subscriber has received
its message. All QoS functionality is handled by the broker.

MQTT has the notion of message-retention, i.e., a message can be retained on a
topic and delivered to any future subscribers. Only a single message is retained
on a given topic at a time. Publishing an MQTT-message with an empty (zero
bytes) payload to a topic will clear its previously retained message.

In Unify, all message payloads are JSON-objects. Some of the Unify topics (e.g.,
commands) may not require any data, but they will require at least an empty JSON
object (e.g., {}).

### Communication Use Cases (Advanced)

The purpose of this section is to give advanced users information about the
communication flow between the Unify components and the physical IoT devices. A
couple of use cases are provided with sequence diagrams describing the frame
flow. The examples require the Mosquitto command-line utilities to be installed.

#### Including a Device {#Including_a_Device}

In the following example, the message flow for including a non secure Z-Wave
device is considered.

To start with, locate the Z-Wave Protocol Controller. All protocol controllers
can be found by subscribing to /ProtocolController/NetworkManagement and the
/State MQTT topics.

``` sh
mosquitto_sub -v -t ucl/by-unid/+/ProtocolController/NetworkManagement -t ucl/by-unid/+/State
```

On your terminal, you will see something similar to the following:

``` txt
ucl/by-unid/zw-C6739552-0001/ProtocolController/NetworkManagement {
    "State": "idle"
}

ucl/by-unid/zw-C6739552-0001/State {
    "NetworkStatus": "Online Functional",
    "Security": "Z-Wave S2 Access Control",
    "MaximumCommandDelay": "0"
}
```

Here, only one controller is connected to the broker. The unique ID (unid) of
the controller is _zw-C6739552-0001_. You can see from the _zw_ prefix that this
is a Z-Wave controller where the Z-Wave home ID of the controller is C6739552
and the controller node ID is 1.

Keep the `mosquitto_sub` command running. Now, you will put the Z-Wave
controller in inclusion mode using another terminal:

``` sh
mosquitto_pub -t ucl/by-unid/zw-C6739552-0001/ProtocolController/NetworkManagement/Write -m '{"State":"add node"}'
```

First, you see that the ProtocolController/NetworkManagement topic changes to
the following:

``` txt
ucl/by-unid/zw-C6739552-0001/ProtocolController/NetworkManagement {
    "State": "add node"
}
```

When you activate the device, a new /State topic appears, as follows:

``` txt
ucl/by-unid/zw-C6739552-0003/State {
    "NetworkStatus": "Online Interviewing",
    "Security": "None",
    "MaximumCommandDelay": "0"
}
```

Shortly after, you see the /State topic and ProtocolController/NetworkManagement
topics change.

``` txt
ucl/by-unid/zw-C6739552-0003/State {
    "NetworkStatus": "Online functional",
    "Security": "None",
    "MaximumCommandDelay": "0"
}

ucl/by-unid/zw-C6739552-0001/ProtocolController/NetworkManagement {
    "State": "idle"
}
```

The device is now included.

### Turning on a Light

After this device is included, it can be operated by sending the OnOff
cluster commands. For example, the following topic and payload will turn on the
light:

``` sh
mosquitto_pub -t ucl/by-unid/zw-C6739552-0003/Commands/OnOff/On -m '{}'
```
### Further examples

More examples of MQTT payloads can be found in the
[UCL MQTT Reference](https://siliconlabs.github.io/UnifySDK/reference_ucl_mqtt/html/index.html)

## System Requirements

The Unify SDK is at the moment only implemented for the Linux OS. The table
below shows the resource usage of the individual components measured on armhf release binaries. No debug symbols included. RAM figures are indicative and measured using `pmap`.
>
>- Release binaries are compiled with `-O2 -s`
>- RAM size = stack + heap + .text + .rodata

| Component          | Flash   | RAM                          |
| ------------------ | ------- | ---------------------------- |
| libuic             | 6551 kB | NA                           |
| uic-aoxpc          |  474 kB |  800 kB + 60 B per asset tag |
| uic-zigpc          | 1122 kB | 2000 kB + 8 kB per node      |
| uic-zpc            | 3054 kB | 3200 kB + 8 kB per node      |
| uic-gms            |  410 kB |  680 kB                      |
| uic-nal            |  420 kB |  630 kB                      |
| uic-positioning    |  270 kB |  500 kB                      |
| uic-upti-cap       |  446 kB |  650 kB                      |
| uic-image-provider |  426 kB |  650 kb                      |
| uic-upvl           |  482 kB | 1000 kb                      |
| uic-dev-gui        |  100 MB | 1000 kb                      |

The SDK depends on the following debian packages:

- libedit2
- libsqlite3-0
- libmosquitto1
- libyaml-cpp0.6
- libboost-log1.67.0
- libboost-program-options1.67.0

## Unify SDK Release Notes

The SDK release notes includes chronologically ordered list of changes from the
oldest release to the newest. All the relese notes can be found [here](./release_notes.md).

## Unify SDK Developer Guides

The developer guides for the Unify SDK includes two developer guides. One
general guide called the [Unify developer guide](./doc/readme_developer.md).
In addition, a general documented overview of the Unify SDK Library can be found
[here](./doc/overview.md).
## FAQ

Frequently asked questions for the Unify SDKcan be found in the FAQ section [here](./FAQ.md).

## License

**Copyright 2020 Silicon Laboratories Inc. www.silabs.com**

The licensor of this software is Silicon Laboratories Inc. Your use of this
software is governed by the terms of Silicon Labs Master Software License
Agreement (MSLA) available at
www.silabs.com/about-us/legal/master-software-license-agreement. This software
is distributed to you in Source Code format and is governed by the sections of
the MSLA applicable to Source Code.

## Third party libraries and code

| Name          | Usage                                               | Link                                                                                  | License       |
| ------------- | --------------------------------------------------- | ------------------------------------------------------------------------------------- | ------------- |
| Mosquitto     | for MQTT communication                              | <https://mosquitto.org>                                                               | EPL/EDL       |
| Contiki       | This is used to implement message queues and timers | <https://github.com/contiki-ng/contiki-ng/wiki>                                       | 3-clause BSD  |
| Editline      | for console UI                                      | <https://sourceforge.net/projects/libedit/>                                           | BSD           |
| Sqlite        | for persistent storage                              | <https://www.sqlite.org/copyright.html>                                               | public domain |
| Boost         | logging, data structures, config files and json     | <https://www.boost.org/LICENSE_1_0.txt>                                               | Boost         |
| nlohmann/json | json                                                | <https://github.com/nlohmann/>                                                        | MIT           |
| CMock         | Unittest                                            | <https://github.com/ThrowTheSwitch/CMock/blob/master/LICENSE.txt>                     |               |
| Unity         | Unittest                                            | <https://github.com/ThrowTheSwitch/Unity/blob/master/LICENSE.txt>                     |               |
| React         | User interface                                      | <https://github.com/facebook/react/blob/master/LICENSE>                               | MIT           |
| React License | User interface                                      | <https://github.com/react-icons/react-icons/blob/master/packages/react-icons/LICENSE> | Misc *TBD*    |
