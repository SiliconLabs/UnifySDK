# Unify SDK Overview

Latest version of the Unify SDK Overview is available online
[here](https://siliconlabs.github.io/UnifySDK/userguide/html/index.html).

The Unify SDK provides software source and binary packages for Raspberry Pi 4 to
help build an IoT gateway product. The Unify SDK enables an IoT service to control and
manage end nodes in several wireless PHY radios (e.g., Z-Wave, Zigbee, and so on)
supported by Silicon Labs. The Unify SDK uses the Unify Controller Language (UCL) as an
internal abstraction layer to seamlessly control various end nodes
that are enabled with multiple wireless PHY radios.
The detailed architecture of the Unify SDK is discussed in the Unify SDK Specification.

The Unify SDK uses Unify SDK library to decouple API modules from the PHY drivers.
This facilitates code reuse and enables easily adding new
high-level APIs without the need to modify the PHY drivers. 
The library provides a number of components that simplify the task of writing a PHY driver
also known as a Protocol Controller.

## Applications References

- ANGEL
  - [User guide](applications/angel/readme_user.md)
  - [Developer guide](applications/angel/readme_developer.md)
  - [Release notes](applications/angel/release_notes.md)
- Dev GUI
  - [User guide](applications/dev_ui/dev_gui/readme_user.md)
  - [Developer guide](applications/dev_ui/dev_gui/readme_developer.md)
  - [Release notes](applications/dev_ui/dev_gui/release_notes.md)
- OTA Image Provider
  - [User guide](applications/image_provider/readme_user.md)
  - [Developer guide](applications/image_provider/readme_developer.md)
  - [Release notes](applications/image_provider/release_notes.md)
- UPVL
  - [User guide](applications/upvl/readme_user.md)
  - [Developer guide](applications/upvl/readme_developer.md)
  - [Release notes](applications/upvl/release_notes.md)
- ZigPC (Beta)
  - [User guide](applications/zigpc/readme_user.md)
  - [Developer guide](applications/zigpc/readme_developer.md)
  - [Release notes](applications/zigpc/release_notes.md)
- ZPC
  - [User guide](applications/zpc/readme_user.md)
  - [Developer guide](applications/zpc/readme_developer.md)
  - [Release notes](applications/zpc/release_notes.md)

## API References

All of these API references points to latest version online. Specific versions
of the APIs are located in the docs folder in the source repository.

- [ANGEL](https://siliconlabs.github.io/UnifySDK/uic-angel/doc_uic-angel/uic_angel/index.html)
- [Unify SDK OTA Image Provider](https://siliconlabs.github.io/UnifySDK/uic-image-provider/doc_uic-image-provider/uic_image_provider/index.html)
- [Unify SDK Library](https://siliconlabs.github.io/UnifySDK/doxygen_uic/html/index.html)
- [UPVL](https://siliconlabs.github.io/UnifySDK/uic-upvl/doc_uic-upvl/uic_upvl/index.html)
- [ZigPC](https://siliconlabs.github.io/UnifySDK/doxygen_zigpc/html/index.html)
- [ZPC](https://siliconlabs.github.io/UnifySDK/doxygen_zpc/html/index.html)
- [ZPC libs2](https://siliconlabs.github.io/UnifySDK/libs2/html/index.html)

## Unify SDK Components

A Unify gateway consists of a Message Queuing Telemetry Transport (MQTT) broker
and a number of MQTT clients. The Unify SDK uses the Mosquitto MQTT broker.

The Unify MQTT clients can be grouped into several applications:

- Z-Wave Protocol controller (ZPC)
- Zigbee Protocol Controller (ZigPC)
- Unify SDk Smart Start Provisioning list (UPVL)
- Unify SDK Multicast Group Manager (ANGLE)
- Unify SDK OTA Image Provider (Image Provider)
- IoT Services (i.e., DEV GUI)

![SDK Overview](doc/assets/img/SDKOverview.svg)

The Unify SDK was previously known as Unified IoT Controller or UIC for short.
The old name can still be found in some parts of the SDK.

### Core Unify Components

The Unify SDK contains the following core components:

#### Protocol Controllers

The Unify SDK currently includes two protocol controllers:

- Z-Wave Protocol Controller ([ZPC](applications/zpc/readme_user.md))
- Zigbee Protocol Controller (Beta) ([ZigPC](applications/zigpc/readme_user.md))

Each protocol controller interfaces with its radio hardware and
implements a translation between its own wireless protocol and the
_Unified Command Language_ (UCL), which is defined in the Unify SDK Specification.
In addition, these protocol controllers implement best practices regarding
message delivery, and adherence to regulatory requirements. Protocol-specific
implementation details can be found in the user guide for each protocol controller.

#### The Unified SDK Provisioning List (UPVL)

[The UPVL](applications/upvl/readme_user.md) serves the Smart Start Provisioning
list to perform Smart Start Security 2 (S2) inclusions and maintains the ucl/SmartStart MQTT topic.

#### The Unify SDK Group Manager (ANGEL)

[ANGEL](applications/angel/readme_user.md) is an application that manages groups and bookkeeping between protocol controllers, and also publishes group
state changes to the ucl/by-group MQTT topic.

#### The Unify SDK OTA Image Provider

[OTA Image Provider](applications/image_provider/readme_user.md) is an application that announces OTA
images available in OTA storage and publishes OTA binary on request.

### IoT Services

Currently, one IoT service is provided, as follows:

- [Developer UI (dev_ui)](applications/dev_ui/dev_gui/readme_user.md)

The dev_ui service is a graphical user interface provided for
operating and provisioning IoT devices using UCL. The dev_ui can be used as
a reference for basic Unify concepts, such as device provisioning and control.
The user interface is provided strictly for test and demonstration purposes only
and is not suitable for production.

## Getting Started

This section describe how to install the Unify SDK components on a Raspberry Pi 4 (RPi4)
and test basic functionality.

### Prerequisites

- An RPi4 preinstalled with Debian Linux Version 10 ('Buster')
- One of the following, supported wireless radios is connected to the RPi4 via USB:
  * UZB 7 serial dongle for Z-Wave
  * EFR32MG12 running NCP Firmware for ZigBee

If these prerequisites are not fulfilled, follow the setup instructions in the [Official Raspberry Pi User Guide](https://projects.raspberrypi.org/en/projects/raspberry-pi-setting-up),
and ensure a supported radio module is attached to your RPi4.

### Installation

First, copy the Unify SDK to your RPi4 and login via ssh:

``` bash
scp uic-sdk_<version>.zip pi@<pi_address>:/home/pi
ssh pi@<pi_address>
```

<version> is the current Unify release version, and <pi_address> is the address of your RPi4.


Once logged into the Raspberry Pi, the SDK must be extracted using the following unzip
command:

``` bash
unzip uic-sdk_<version>.zip
```

The easiest way to install the Unify components on the Raspberry Pi is to install
all packages using the apt command as this will also fetch dependencies:

``` bash
curl -s http://repo.mosquitto.org/debian/mosquitto-repo.gpg.key | sudo apt-key add -
sudo curl -s  http://repo.mosquitto.org/debian/mosquitto-buster.list -o /etc/apt/sources.list.d/mosquitto-buster.list
sudo apt update
sudo apt install mosquitto mosquitto-clients
cd uic-sdk_<version>
sudo apt install ./deb-packages/*.deb
```

Use `sudo apt reinstall ./deb-packages/*.deb` to update previously installed
packages to a newer version. Repeated use of `apt install` has been known to
skip updating some packages.

### Configuration

If this is the first time the Unify SDK is installed, you will be prompted with
configuration options.

After setting the first-time configuration options, configure system to restart Unify components automatically on every boot with the following commands:

``` bash
sudo systemctl enable uic-zpc
sudo systemctl start uic-zpc
```

### Evaluation

The Unify dev_ui can be accessed from a browser on [http://raspberrypi.local:3000](http://raspberrypi.local:3000).

_Note_ that the Unify user interface needs TCP access to the port 3000 and 1337 on
the Raspberry Pi.

See the [Dev-UI manual](applications/dev_ui/dev_gui/readme_user.md)
for more information about using this interface.

## Communication Overview

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

## Communication Use Cases (Advanced)

The purpose of this section is to give advanced users information about the
communication flow between the Unify components and the physical IoT devices. A
couple of use cases are provided with sequence diagrams describing the frame
flow. The examples require the Mosquitto command-line utilities to be installed.

For a more basic introduction, see the Dev-UI.

### Including a Device {#Including_a_Device}

In the following example, the message flow for including a non
secure Z-Wave device is considered.

To start with, locate the Z-Wave Protocol Controller. All protocol
controllers can be found by subscribing to /ProtocolController/NetworkManagement
and the /State topictMQTT topics.

``` bash
mosquitto_sub -v -t ucl/by-unid/+/ProtocolController/NetworkManagement -t ucl/by-unid/+/State
```

On your terminal, you will see something like the following:

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

Here, only one controller is connected to the broker. The unique ID (unid)
of the controller is _zw-C6739552-0001_. You can see from the _zw_ prefix that
this is a Z-Wave controller where the Z-Wave home ID of the controller is C6739552
and the controller node ID is 1.

Keep the `mosquitto_sub` command running. Now, you will put the Z-Wave controller
in inclusion mode using another terminal:

``` bash
mosquitto_pub -t ucl/by-unid/zw-C6739552-0001/ProtocolController/NetworkManagement/Write -m '{"State":"add node"}'
```

First, you see that the ProtocolController/NetworkManagement topic changes to the following:

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

Shortly after, you see the /State topic and
ProtocolController/NetworkManagement topics change.

```
ucl/by-unid/zw-C6739552-0003/State {
    "NetworkStatus": "Online functional",
    "Security": "None",
    "MaximumCommandDelay": "0"
}

ucl/by-unid/zw-C6739552-0001/ProtocolController/NetworkManagement {
    "State": "idle"
}
```

The device has now been included.

### Turning on a Light

After this device has been included, it can be operated by sending the OnOff
cluster commands. For example, the following topic and payload will turn on the
light:

``` bash
mosquitto_pub -t ucl/by-unid/zw-C6739552-0003/Commands/OnOff/On -m '{}'
```

## How To Make A New IoT Service {#How_to_make_a_new_IoT_Service}

To build an IoT Service from scratch, it is recommended to follow the steps
below. IoT Services come in many forms, and exact steps will depend on the
detailed requirements of the IoT Service.

1. Find API and SDK bindings for target service. An IoT service connects the Unify
   SDK with an external service. Service should be interpreted in the broadest
   sense; it can be a cloud service, or it can be a UI operated by end users. Or
   something in between. The first step in writing an IoT Service is determining
   which APIs and SDKs to use for the target service. If the IoT Service
   connects to a cloud service, then the cloud service SDK is a good place to
   start. Alternatively, choosing a framework with good bindings to the cloud
   service. If the IoT Service is a UI, then a UI framework should be chosen
   based on the product requirements.
2. Choose an MQTT client. Now choose an MQTT client based on the programming
   language and frameworks selected in the previous step. Most MQTT clients
   should be compatible with the Unify SDK. If encryption is desired, make
   sure to choose a client that supports certificate-based TLS.
3. Discover nodes using MQTT topics. Discovering nodes and resources that can be
   displayed or manipulated is fundamental to any IoT Service. This is achieved
   by subscribing to particular MQTT topics. See the Service Discovery chapter
   of the Unify Specification.
4. Implement Network Management. If the IoT Service needs to do network
   management, consider which operations are needed and implement them as
   described in the Network Management chapter of the
   Unify Specification. Here is
   [an example of adding a node to a network](#Including_a_Device).
5. Interact with nodes using MQTT topics. Interacting with the nodes by either
   reading out status or controlling their state is fundamental to most IoT
   services. Subscribe and publish to the relevant MQTT topics as described in
   the Application Level Control chapter of the
   Unify Specification.
6. Consider re-using appropriate shared components from the Unify SDK codebase.
   A valuable feature of Unify is the common components shared by many modules
   of the SDK. Code reuse saves both development time and offers IoT
   Service users a uniform experience operating and configuring different IoT
   services.

The Unify SDK provides a nubmer of C-based components that are candidates
for re-use in IoT Services. They are listed below.

1. Unify MQTT Client. The MQTT client is built on top of Eclipse Mosquitto MQTT
   and handles MQTT connection, subscribing and publishing. It implements
   re-connections and caching of published messages, which have been published
   while the client has been disconnected from the broker. See the section Unify
   Library Framework/MQTT Client in Unify Documentation for
   details.
2. Logging System. The logging system features logging with severity level
   tagging and log filtering. See the section Unify Library Framework/Logging
   System in the Unify Documentation for details.
3. Config System. The config system implements parsing of config files and
   command line arguments. See the Unify Library Framework/Config System in
   Unify Documentation for details.

The UPVL and ANGEL applications use the shared components and are good examples
to peruse. They can be found in the Unify source code in the
`applications/angel/` and `applications/upvl/` directories.

## System Requirements

The Unify SDK is at the moment only implemented for the Linux OS.
The table below shows the resource usage of the individual components measured
using [Valgrind](https://valgrind.org/) on a i686 architecture, compiled without
debugging information.


|     Component     |  Flash  |         RAM             |
|-------------------|---------|-------------------------|
| libuic            | 4085 kb |                 NA      |
| uic-zigpc         | 1880 kb |  1900 kb + 8 kb pr node |
| uic-zpc           | 5888 kb |  1500 kb + 8 kb pr node |
| uic-angel         | 3220 kb |                1000 kb  |
| uic-image-provider| 3281 kb |                1000 kb  |
| uic-upvl          | 3339 kb |                1000 kb  |
| uic-dev-gui       |   95 MB |                1000 kb  |

The SDK depends on the following debian packages:

  - libedit2
  - libsqlite3-0
  - libmosquitto1
  - libyaml-cpp0.6
  - libboost-log1.67.0
  - libboost-program-options1.67.0

## Porting the Unify SDK

Porting the Unify SDK to new platform using Debian Linux should be straight forward.
However, if components are ported to other Linux distributions it may be
necessary to update the startup scripts of the components. The Debian packages
provided with the Unify SDK all uses the system service `systemd` which takes
care of startup and shutdown of the components. For example, the file
[uic-zpc.service](applications/zpc/scripts/systemd/uic-zpc.service) is as
systemd config script. When porting to a new Linux system the systemd config
files should be used for reference. It is very important that all the Unify
components runs as a dedicated system user to prevent attackers from
compromising the entire system. Also note that most components need write
access to parts of the filesystem. The default path for writing files is
`/var/lib/uic`. 
## FAQ
* *How can I see if the Unify components are running?*<br>
Unify applications are running as Linux `systemd` services. To get a status overview of all installed Unify components, run the following command:<br>
```sh
systemctl status `ls /lib/systemd/system/uic-*.service | xargs -n 1 basename`
```
The output of this command will tell if the service is loaded and/or is active:  
```sh
● uic-dev-gui-api.service - Unify User Interface API
   Loaded: loaded (/lib/systemd/system/uic-dev-gui-api.service; enabled; vendor preset: enabled)
   Active: active (running) since Sun 2021-07-25 12:17:05 BST; 21min ago
```
* *How do I get logging of a specific Unify component?*<br>
Logging is collected via Linux `systemd`. In the terminal `journalctl` is able to print the collected logs from all Unify components. Getting the log of, for instance `uic-upvl`, is done via command:
```sh
journalctl -u uic-upvl
``` 
* *How do I clear Unify caches and databases?*<br>
Execute the following command in a terminal on the Unify gateway:
```sh
unify_services=`ls /lib/systemd/system/uic-*.service | xargs -n 1 basename`; sudo systemctl stop mosquitto $unify_services && sudo rm /var/lib/mosquitto/mosquitto.db /var/lib/uic/database.db /var/lib/uic/upvl.db && sudo systemctl start mosquitto $unify_services
```
Note: database paths can vary depending on your configuration
* *I cant access the Dev UI/ the Dev UI is not responding anymore!*<br>
1. Test if the gateway can still be reached over TCP/IP. Ping the ip address of the gateway.
1. Open a terminal on the Unify gateway, check the current status of the `uic-dev-gui-api` and `uic-dev-gui-server`. (See previous questions how to print the status)<br>
2. If status is inactive, restart the UI:
```sh
sudo systemctl restart uic-dev-gui-*
```
* *My Z-Wave device doesn't seem to include with my gateway*
1. Make sure your device and gateway match frequency region. i.e., an EU device is incompatible with an US gateway. Information of the region can be find on the packaging of the device.<br>
To verify the rf region on your gateway, execute the following command in your gateway terminal:
```sh
cat /etc/uic/uic.cfg
```
It outputs the configured rf_region:
```bash
pi@raspberrypi:/etc/uic $ cat uic.cfg 
zpc:
  serial: /dev/ttyUSB0
  rf_region: US
datastore:
  file: /var/lib/uic/database.db
```
2. Make sure your device isn't still connected to a previous network. To remove a previous inclusion, hit the action button on your product and put the gateway in remove state.
3. Power-cycling your device could make it to include.
4. To change the frequency region of the gateway, see the startup options for the ZPC.
* *The gateway doesn't respond anymore!*<br>
1. It could be that the gateway experiences some delays while communicating with the Z-Wave node (missing Acknowledgment/Report, routing issues...). In this case, it may look like the gateway does not react to IoT Services commands. You could verify that the gateway still responds by modifying the state of another Z-Wave device in the gateway and see if the gateway responds to these changes. A way to modify state is to open MQTT explorer and to publish a command to a cluster for a particular node.
2. As last resort, try to restart the Unify gateway.
* *How do I run the ZPC in isolation?*<br>
Stop the ZPC systemd service:
```sh
sudo systemctl stop uic-zpc
```
run:
```sh
/usr/bin/zpc --zpc.serial /dev/ttyUSB0
```
Replace the serial argument with the port the Z-Wave controller is connected to.
* *I have an old Z-Wave device, is it still supported?*<br>
Yes. However, older devices may be inconsistent in reporting their actual state. This usually means that operating the same command for a second time, the state get updated correctly. 
* *Can I get detailed diagnostics about devices, its clusters and the network?*<br>
Using application [`mqtt explorer`](http://mqtt-explorer.com) can give more insights in your Unify network. 
* *How do I determine the root cause for my failed firmware update?*<br>
The lastError field in the OTA tab of the dev_gui is giving a rough error code. The logging of the uic-zpc usually gives a more detailed explanation of the reason why a firmware update is not accepted.
* *My device shows up with more clusters than expected!*<br>
This is intended behavior. A protocol cluster can overlap in functionality with multiple UCL clusters. Rather then mapping to one, The Unify gateway exposes the protocol cluster to multiple clusters.
 ### Development
* *Can I build with Rust's cargo manually?*<br>
Yes. use the following command in the project root dir:
```sh
LIBUIC_DIR=/Users/unifyuser/git/uic/build/components VERSION_STR="local_build" cargo build
```
Change LIBUIC_DIR to match your path.
* *Is mac supported for developing?*<br>
Our software should build and run on a mac machine. However, it is not officially supported.
### ZPC
* *ZPC complains about etc/uic/database.db permissions, How do I fix it?*<br>
This means your probably running the ZPC as an executable and not as a `systemd` service. Advised is to run ZPC with a database path that is writable in this case:
```sh
/usr/bin/zpc --datastore.file ~/zpc.db
```
### UPVL - ANGEL - IMAGE_PROVIDER
* *Inclusion of a device keeps failing*
try to reset the UPVL database:
```
sudo systemctl stop uic-upvl && rm /var/lib/uic/upvl.db && sudo systemctl start uic-upvl
```
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
| CMock         | Unittest                                            | <https://github.com/ThrowTheSwitch/CMock/blob/master/LICENSE.txt>                     |               |
| Unity         | Unittest                                            | <https://github.com/ThrowTheSwitch/Unity/blob/master/LICENSE.txt>                     |               |
| React         | User interface                                      | <https://github.com/facebook/react/blob/master/LICENSE>                               | MIT           |
| React License | User interface                                      | <https://github.com/react-icons/react-icons/blob/master/packages/react-icons/LICENSE> | Misc *TBD*    |

