# ZPC User's Guide

ZPC translates the Unify Controller Language(UCL) into Z-Wave commands and
communicates with Z-Wave devices in a Z-Wave network.

## Installation

The ZPC is installed on a Debian Buster platform using the uic-zpc Debian
package provided with the SDK.

```bash
apt install uic-zpc_<version>_armhf.deb
```

### Files

The Debian package of the ZPC contains the following files:

| Path                                       | Description                       |
| ------------------------------------------ | ----------------------------------|
| /lib/systemd/system/uic-zpc.service        | Systemd service file              |
| /usr/bin/zpc                               | ZPC application                   |
| /usr/share/bash-completion/completions/zpc | script for bash auto completion   |
| /usr/share/doc/uic-zpc/copyright           | copyright notice                  |
| /etc/uic/uic.cfg                           | Default location of config file   |
| /var/lib/uic/database.db                   | Default location of database      |
| /usr/share/uic/rules                       | Default location of mapping rules |

#### Configuration File

The configuration file is written in YAML and is used to set up the ZPC.
The ZPC can dump the running configuration, which can be used as a starting
point for the config file.

Configuration file example:

```bash
$ zpc --dump-config
# Unify sample conf file

datastore:
  - file:'/var/lib/uic/database.db'
log:
  - level:'d'
  - tag_level:'uic_main:debug'
mapdir: '/usr/share/uic/rules'
mqtt:
  - host:'localhost'
  - port:1883
zpc:
  - accepted_transmit_failure:2
  - default_wakeup_interval:4200
  - device_id:'36833760FCEF5E5C8E66077764918592'
  - hardware_version:1
  - manufacturer_id:0
  - measured_0dbm_power:0
  - missing_wakeup_notification:2
  - normal_tx_power_dbm:0
  - product_id:1
  - product_type:1
  - rf_region:'EU'
  - serial:'/dev/ttyUSB0'
  - serial_log_file:''
```

#### Database File

The ZPC uses a sqlite database for storing network information. The default
location of the database is /var/lib/uic/database.db. The database must be located
in a writable part of the file system. If the database is lost, the ZPC will lose
vital information about the network and need to probe all nodes in the network.

## Running ZPC

You can run the ZPC either by using the
systemd service, that is installed through the Debian package, or by
running the ZPC from the command line.

### Running ZPC Using Systemd

Normally, the ZPC, which is installed as a systemd service, should be run as a
service.

You can start and stop the ZPC service with the
systemctl command (see systemctl linux manual for details).

```bash
sudo systemctl start uic-zpc
sudo systemctl stop uic-zpc
```

To see the logs of the ZPC when it runs as a service, use the following command:

```bash
journalctl -u uic-zpc.service
```

### Running ZPC From the Command Line

Running the ZPC from the command line provides a Command Line Interface (CLI)
that supports various debugging commands. To get more information about the
commands, type `help` in the CLI.

These are the steps to run the ZPC from the command line:

1. Ensure Mosquitto broker is running, either by systemd `sudo systemctl start
   mosquitto` or by running `mosquitto -d` in a terminal.
2. Run `/usr/bin/zpc`.

> To get more information about command line arguments for ZPC, run `/usr/bin/zpc --help`.

Note that the default permissions for the ZPC database file located in
/var/lib/uic/database.db is only writable for the _uic_ system user (which is
created upon installation). If you run the zpc as another user,
select a different database location.

For example,

```bash
zpc --zpc.serial /dev/ttyUSB0 --zpc.rf_region EU --datastore.file mydatabase.db --mapdir rules
```

#### (Advanced) Configuring ZPC to Use TLS Certificate-Based Encryption
##### Self Signed Certificate Generation

The following script can generate and self sign the necessary certificates for connection ZPC to
MQTT broker in the directory format shown below.

```bash
scripts/tls-certificates-generation.sh <broker IP/hostname>
```
Default: localhost (MQTT broker and ZPC are running on the same machine)

```
$ tree .
├── broker
│   ├── broker.crt
│   └── broker.key
├── ca
│   ├── ca.crt
│   ├── ca.key
│   └── ca.srl
└── client
    ├── client.crt
    └── client.key
```

Passing `--mqtt.cafile (ca/ca.crt)`, `--mqtt.certfile (client/client.crt)` and
`--mqtt.keyfile (client/client.key)` enables the TLS support in ZPC.

Having the config options: `"cafile" (client/ca.crt)`, `"certfile"
(broker/broker.crt)` and `"keyfile" (broker/broker.key)` in Mosquitto Broker
config file will make Broker communicate with ZPC with TLS encryption.

##### Broker Configurations

Configure the Mosquitto broker with the following parameters to enable
TLS support. Broker can support TLS and non-TLS communication simultaneously
on two different ports. For example, in the following mosquitto.conf, Mosquitto Broker
will accept non-TLS communication on port 1883.

While on port 8883 Mosquitto Broker will need ZPC to pass correct --mqtt.cafile,
--mqtt.certfile and --mqtt.keyfile on command line to connect to TLS encrypted
communication with ZPC.

This will allow UPVL and DEV_GUI and other MQTT Clients to have non-DTLS
connection to Broker on port 1883 while ZPC can have TLS connection to Broker
on port 8883.

###### mosquitto.conf

```bash
port 1883
allow_anonymous true
per_listener_settings true

listener 8883
cafile ca/ca.crt
certfile broker/broker.crt
keyfile broker/broker.key
require_certificate true
```

##### ZPC command line options

###### CA File

Path to file containing the PEM encoded CA certificate to connect to Mosquitto
MQTT broker for TLS encryption
```bash
--mqtt.cafile arg
```

###### Client Certificate File

Path to file containing the PEM encoded client certificate to connect to
Mosquitto MQTT broker for TLS encryption

```bash
  --mqtt.certfile arg
```

###### Client Private Key File

Path to file containing the PEM encoded CA certificate to connect to Mosquitto
MQTT broker for TLS encryption

```bash
--mqtt.keyfile arg
```

Note: ZPC does not support encrypted keyfile

TLS support example:

```bash
zpc --zpc.serial /dev/ttyUSB0 --zpc.rf_region EU --datastore.file mydatabase.db --mapdir rules --mqtt.cafile ca/ca.crt --mqtt.certfile client/client.crt --mqtt.keyfile client/client.key
```

When the ZPC starts, the log is printed to stdout, as follows:

```
+ 2020-11-11 14:18:52:276827 <d> [sl_log] Setting log level to debug
+ 2020-11-11 14:18:52:277573 <d> [uic_component_fixtures] Completed: UIC Signal Handler
+ 2020-11-11 14:18:52:277631 <d> [mqtt_client] Constructing new mqtt_client instance.
+ 2020-11-11 14:18:52:277696 <d> [mqtt_wrapper_mosquitto] Initializing Mosquitto library.
```

The general log format is as follows:

```
<timestamp> <severity> [tag] <message>
```

The log level can be set globally and pr tag.

For example:

```bash
zpc  --log.level i --log.tag_level uic_component_fixtures:d,mqtt_client:d
```

This will set the global log level to info but it enables the debug on the two
components such as the following: mqtt_client and uic_component_fixtures.

## Understanding how the ZPC Works

The purpose of this section is to explain the ZPC behavior and how to correlate
frames seen on a Z-Wave Zniffer with operations performed by the ZPC.

The ZPC performs network discovery and operation of for both listening and non
listening devices using a software component called the attribute system.

The basic idea of the attribute system is that all network state parameters have
a reported value and a desired value. The following are examples of state
parameters:

* _is the lamp on_
* _is the door locked_
* _current room temperature_
* ...

The reported value of an attribute is data about the current attribute state
that the device has communicated to the ZPC. If the device has not told the ZPC
what the value is, the reported value is unresolved.

The desired value is the user-preferred attribute value, such as _user wants the
light to be off_. Note that it does not make sense for all attributes to have a
desired value. In other words, a smoke detector should not have a desired value
to prevent a user from turning if off or setting the smoke to go away.

The ZPC attribute system is used to resolve an unresolved attribute using a set
of rules. For instance, you can get the binary switch value of a Z-Wave Binary
Switch node by sending the Z-Wave BINARY_SWITCH_GET command. If the attribute
system notices that the desired value of an attribute is different than the
reported value, the ZPC will issue a Z-Wave SET command.

> Note that the ZPC will only send a SET command if it notices that the reported
> value of an attribute is different than the desired. The ZPC will do its best
> to make sure that it has the right reported value by setting up Lifeline
> associations, but there may be situations where the ZPC has outdated
> information.

If a node supports supervision, the ZPC will use supervision encapsulation when
sending a SET command. In this way, the ZPC will update the reported value
automatically when sending a SET command. If a node does not support
supervision, the ZPC will mark the reported value of an attribute unresolved
when sending a SET and this will trigger a new GET command for the attribute to
update the reported value.

> Currently, when the ZPC sends a SET command with supervision encapsulation and
> the device reports the supervision state working, the ZPC will not reflect
> this on the MQTT side. The reported value will be updated on MQTT side when
> the supervision state OK/FAILED is received.

The Z-Wave Wake up devices are supported by pausing node resolution when
the device is sleeping, which means that if the temperature of a thermostat is
changed multiple times while the device is sleeping, the ZPC will only send the
latest change using thermostat setpoint command. The ZPC will send wake up
no-more information when there is nothing
more to resolve regarding the node.

## Dotdot Cluster to Z-Wave Command Class Mapping

The ZPC maps dotdot clusters to Z-Wave command classes and vice versa.
All maps are defined by a set of configuration files which the ZPC loads as boot.
However, default files are installed in the folder /usr/share/uic/rules. The mapping
file extension is *.uam which is short for Unified Attribute Map.

The following table has a brief description of how ZCL clusters are mapped to Z-Wave
command classes.


| DotDot Cluster          | Z-Wave Command Classes                                         | Notes                                                                                                                      |
| ----------------------- | -------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------- |
| OnOff                   | Binary Switch <br> Basic <br> Multilevel Switch                | The DotDot OnOff commands (On and Off) are mapped to Z-Wave Binary Switch Set command  or Basic SET                        |
| OccupancySensing        | Notification                                                   | The Z-Wave Notification Report command is mapped to Occupancy and OccupacyType DotDot attributes                           |
| IASZone                 | Notification                                                   |                                                                                                                            |
| Illumiance level sensing| Notification                                                   |                                                                                                                            |
| DoorLock                | Door Lock                                                       | The DotDot DoorLock commands (LockDoor and UnlockDoor) are mapped to Z-Wave Door Lock Set command                          |
| Thermostat              | Multilevel Sensor<br> Thermostat Setpoint <br> Thermostat Mode | The DotDot SetpointRaiseOrLower and WriteAttribute commands are mapped to Thermostat Setpoint and Thermostat Mode Commands |
| TemperatureMeasurement  | Multilevel Sensor                                              |                                                                                                                            |
| Level                   | Multilevel Switch                                             |                                                                                                                            |
| IASZone                 | Battery                                                        |                                                                                                                            |

## Migrating to ZPC from Z/IP Gateway

This section describes how to migrate to a ZPC from a Z/IP gateway. If migration
from a third party gateway is desired, the first step in the migration process is
to migrate to a Z/IP Gateway.

To successfully migrate, the following Z/IP Gateway SDK tools are needed.
- `zw_programmer` Is required for programming new firmware on the Z-Wave modules
  and for reading and writing the Z-Wave controller module NVM.
- `zw_nvm_converter` this tool is used for converting the Z-Wave NVM between
  protocol versions. The tool can be use to migrate from a 500 series Z-Wave
  chip to a 700 series chip.

You can get Z/IP Gateway through Simplicity Studio. Read the
Z/IP Gateway SDK user guide on how to compile and install the Z/IP gateway on
your platform.

For Z-Wave Long Range support, the ZPC requires a Z-Wave API
controller version 7.16 or higher. For details about migrating
between protocol versions see the Z/IP Gateway manual, _Migration Support Tools_

To ensure that the ZPC is working correctly, it must have access to the network
encryption keys as well has information about which security keys have been
granted to each node in the network. The Z/IP Gateway stores the encryption
keys in the right place in the module NVM for the ZPC to read.

In general, the ZPC will query all network information from the nodes in the
network on the first boot, but it needs to know which network keys have been
granted to the nodes in the network. The Unify SDK contains the tool
`zpc_database_tool`; which can be used for manipulating the ZPC database.
zpc_database_tool can import/export the ZPC database from/to a JSON file.
The easiest way to set the granted key information is to do the following:
1. Migrate the Z-Wave controller using the Z/IP Gateway SDK tools as described
   above.
2. Start the ZPC with the migrated Z-Wave controller module but with an empty
   database file. `zpc --datastore.file datastore.db`
3. Shut down the ZPC by typing `exit` on the console. The ZPC will now have
   created an initial database which matches the Z-Wave network.
4. Use the zpc_database_tool to export the database to a JSON file
   `zpc_database_tool --datastore.file datastore.db --export datastore.json`
5. Open and edit the JSON file and locate the following attributes under each
   ATTRIBUTE_NODE_ID object:
   -  ATTRIBUTE_NETWORK_STATUS must be set to "reported": "00"
   -  ATTRIBUTE_KEX_FAIL_TYPE should be set to "reported": "00"
   -  ATTRIBUTE_S2_DSK this is optional but should be set the DSK of the node.
   -  ATTRIBUTE_NODE_IS_S2_CAPABLE should only be defined if the node supports
      S2
   -  ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL must be set to 1 if the node was
      included using Z-Wave long range and should be 0 otherwise.
   - ATTRIBUTE_ENDPOINT_ID: all endpoint objects besides endpoint 0 must be
     removed. Endpoint 0 should only contain the ATTRIBUTE_ZWAVE_NIF with no
     desired or reported values.
   -  ATTRIBUTE_GRANTED_SECURITY_KEYS is a bitmask of which network keys a node
      have been granted the following keys are currently defined:
      |  Key                                   | Mask |
      |----------------------------------------|------|
      | ZWAVE_CONTROLLER_S0_KEY                | 0x80 |
      | ZWAVE_CONTROLLER_S2_UNAUTHENTICATED_KEY| 0x01 |
      | ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY  | 0x02 |
      | ZWAVE_CONTROLLER_S2_ACCESS_KEY         | 0x04 |

      Ie if a node has all keys the granted keys attribute should be
      `{
      "type": "ATTRIBUTE_GRANTED_SECURITY_KEYS",
      "reported": "87"}
      `

6. With the updated JSON file, a new database can be constructed with zpc_database_tool. `zpc_database_tool --datastore.file datastore.db --import modified_datastore.json`
7. You can now start the ZPC as usual. The ZPC will start probing all nodes, sleeping devices will be probed
  when they wake up.

### ZPC Database JSON Example
```
   {
  "type": "0x00000001",
  "children": [
    {
      "type": "ATTRIBUTE_HOME_ID",
      "reported": "2ab89dc4",
      "children": [
        {
          "type": "ATTRIBUTE_NODE_ID",
          "reported": "0100",
          "children": [
            {
              "type": "ATTRIBUTE_ENDPOINT_ID",
              "reported": "00",
              "children": [
                {
                  "type": "ATTRIBUTE_ZWAVE_NIF",
                }
              ]
            },
            {
              "type": "ATTRIBUTE_NETWORK_STATUS",
              "reported": "01"
            },
            {
              "type": "ATTRIBUTE_GRANTED_SECURITY_KEYS",
              "reported": "9f"
            }
          ]
        },
        {
          "type": "ATTRIBUTE_NODE_ID",
          "reported": "0500",
          "children": [
            {
              "type": "ATTRIBUTE_ENDPOINT_ID",
              "reported": "00",
              "children": [
                {
                  "type": "ATTRIBUTE_ZWAVE_NIF",
                }
              ]
            },
            {
              "type": "ATTRIBUTE_NETWORK_STATUS",
              "reported": "01"
            },
            {
              "type": "ATTRIBUTE_GRANTED_SECURITY_KEYS",
              "reported": "00"
            },
            {
              "type": "ATTRIBUTE_KEX_FAIL_TYPE",
              "reported": "00"
            },
            {
              "type": "ATTRIBUTE_S2_DSK",
              "reported": "123498a7ac59a1b9096f99a0edb95e06"
            },
            {
              "type": "ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING",
              "reported": "d3"
            },
            {
              "type": "ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL",
              "reported": "9c"
            },
            {
              "type": "ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL",
              "reported": "00000000"
            },
            {
              "type": "ATTRIBUTE_NODE_IS_S2_CAPABLE"
            }
          ]
        }
      ]
    }
  ]
}
```


## Performing Backup and Restore

The backup and restore process works under the assumption that the
controller firmware version for the backup and restore is identical.

The following are the backup steps:
- Stop the ZPC.
- Use zw_programmer to back up the NVM of the Z-Wave module.
- Make a copy of the SQLite database used by the ZPC.
- Start the ZPC again.

The process of restoring a backup is following:

- Stop the ZPC.
- Use zw_programmer to restore  the NVM to the Z-Wave module.
Copy a backup of the SQLite database to an active SQLite database.
- Start the ZPC again.


## Performing Firmware Updates

The ZPC has the capability to perform Firmware Updates. It requires to
run and configure the
[Image Provider](../../applications/image_provider/readme_user.md) application.

### UIID Construction

To upload an image for a Z-Wave node, compute a Unique Image Identifier (UIID)
for this device.

The UIID for the ZPC is a string that can be constructed using the
following information:

* The Manufacturer ID (Manufacturer Specific Command Class)
* The Product Type (Manufacturer Specific Command Class)
* The Product ID (Manufacturer Specific Command Class)
* The Firmware Target (Firmware Update Command Class)

The string must be formatted using the following format:
**ZWave-&lt;ManufacturerID&gt;-&lt;ProductType&gt;-&lt;ProductID&gt;-&lt;FirmwareTarget&gt;**

### UIID/UNID Association

When the ZPC interviews a node, it will publish the detected firmware targets
under the UIID space in the OTA cluster:
`ucl/by-unid/<UNID>/ep0/OTA/Attributes/UIID/<UIID>/#`

For more details about available attributes under each UIID, see the
the Unify specification - Common OTA FW Update Service chapter.

```
ucl/by-unid/<UNID>/ep0/OTA/Attributes/UIID/<UIID>/CurrentVersion/Reported
```

For example, if a node has 2 Firmware Targets, the publications may look as follows:

```
ucl/by-unid/zw-DB9E8293-0007/ep0/OTA/Attributes/UIID/ZWave-010f-1002-0b01-0000/CurrentVersion/Reported - {"value": "3.2.0"}
ucl/by-unid/zw-DB9E8293-0007/ep0/OTA/Attributes/UIID/ZWave-010f-1002-0b01-0001/CurrentVersion/Reported - {"value": "3.2.0"}
```

UIID *ZWave-010f-1002-0b01-0000* can be used to firmware update target 0.
UIID *ZWave-010f-1002-0b01-0001* can be used to firmware update target 1.

### Version String Calculation

The version of a firmware is determined based on the data reported by the
Z-Wave node using the Version Command Class.

The version string will be composed of three decimal digits following
[semantic versioning](https://semver.org/). By default, the version
for a given firmware will be fetched in the Version Command Class,
in the Version Report Command. Only the Major and Minor digits can be retrieved
from this report.

For example, if a node sends a Version Report with

* Firmware 0 Version = 3
* Firmware 0 Sub Version = 14

The resulting version string will be *"3.14.0"*. No patch version number is
available.

If a node supports the Version Z-Wave Software Report Command, the version string
for Firmware 0 will be replaced by the Application Version field. For example,
if a node sends a Version Z-Wave Software Report Command with:

* Application Version 1 (MSB) = 3
* Application Version 2 = 14
* Application Version 3 (LSB) = 15

The resulting version string will be *"3.14.15"*.

### Triggering a Firmware Update

The ZPC will download the available images from the
[Image Provider](../../applications/image_provider/readme_user.md) whenever a
matching UIID (and optional UNID) is advertised as available.

To initiate a firmware update, configure the
[Image Provider](../../applications/image_provider/readme_user.md)
with a compatible image.

The ZPC will initiate a firmware update operation immediately and start publishing
the transfer progress in OTA attributes under the UIID space in the OTA cluster.

The ZPC is capable of performing any number of concurrent firmware updates.
To avoid network congestion, configure the [Image Provider](../../applications/image_provider/readme_user.md)
meta data to specify UIID and UNID to control when to apply the firmware of individual nodes.

The *ApplyAfter* parameter can also be used to delay firmware transfers.

### Aborting a Firmware Update

There is currently no way to abort an ongoing Firmware Update.

### After a Successful Firmware Update

The ZPC will not automatically re-interview a node that was OTA firmware updated.
An IoT Service must instruct the ZPC to perform a new node interview to discover
the new capabilities and version information of a node.

### Examples

This sections provides an example using the [Image Provider](../../applications/image_provider/readme_user.md) and [Developer GUI](../../applications/dev_ui/dev_gui/readme_user.md) to perform a firmware update of a PowerStrip sample application.

First, make sure that the ZPC, [Image Provider](../../applications/image_provider/readme_user.md) and [Developer GUI](../../applications/dev_ui/dev_gui/readme_user.md)
are running.

Flash a PowerStrip application on a Z-Wave module with the OTA bootloader
and GBL encryption keys. See the Z-Wave/Gecko SDK documentation in
[Simplicity Studio](https://www.silabs.com/developers/simplicity-studio)

Start the DevGUI and include a PowerStrip (or any other) sample application.
At the end of the interview, the OTA Cluster page will show the UIID of the PowerStrip.

![PowerStrip UIID in DevGUI](./doc/assets/img/powerstrip_uiid_dev_gui.png "PowerStrip UIID")

Prepare a GBL file with a newer firmware version that you upload to the Image
Provider application. It will require several versions of the Z-Wave/Gecko SDK
installed to access binaries with older or newer versions.

You can also perform the operation with an identical firmware image (or older)
but the end node will reject the upgrade operation after the transfer.

Verify that your images are ready and readable:

```bash
pi@unify:/var/lib/uic-image-provider/updates $ ls -l
-rw-r--r-- 1 pi  pi   171380 Aug 17 14:52 powerstrip.gbl
-rw-r--r-- 1 pi  pi   168036 Aug 17 14:52 sensor_pir.gbl
```

Then, use the found UIID to configure the *image.json* file from the image
provider. Remember to set a version string as well as calculate the Md5 as
described in the [Image Provider User Guide](../../applications/image_provider/readme_user.md):

```json
{
  "Version": "1",
  "Images": [
    {
      "FileName": "updates/powerstrip.gbl",
      "Uiid": "ZWave-0000-0005-0004-0000",
      "Unid": ["zw-DB9E8293-0002"],
      "Version": "10.16.4",
      "ApplyAfter": "2021-06-29T16:39:57+02:00",
      "Md5": "4XbdcGq2iXOD1EcZ905XxQ=="
    }
  ]
}
```

Shortly after, the Image Provider will anounce the image(s) over MQTT and the
ZPC will download this image, if some nodes in its network have matching UIIDs.
The Dev GUI will show the Image Provider announcement under the "Images" tabs
in the OTA cluster page:

![PowerStrip UIID image available in DevGUI](./doc/assets/img/powerstrip_image_available_dev_gui.png "PowerStrip UIID image available")


As soon as the ApplyAfter timestamp is passed, you will be able to observe
the firmware transfer operation under the DevGUI:

![PowerStrip firmware transfer in DevGUI](./doc/assets/img/dev_gui_powerstrip_firmware_transfer.png "PowerStrip firmware transfer in DevGUI")


When the firmware update is finished and successful, it will look as follows:

![PowerStrip firmware transfer completed in DevGUI](./doc/assets/img/dev_gui_powerstrip_firmware_transfer_complete.png "PowerStrip firmware transfer completed in DevGUI")

The following conditions have to be met for a successful firmware transfer:

* The Size/Offset attributes are non-zero and set to the same value (The DevGUi will show 100%)
* The LastError attribute is set to "Success"
* The Status attribute is back to "Idle"

for example:

```txt
ucl/by-unid/<UNID>/ep0/OTA/Attributes/UIID/<UIID>/Offset/Reported - {"value": 171380}
ucl/by-unid/<UNID>/ep0/OTA/Attributes/UIID/<UIID>/Size/Reported - {"value": 171380}
ucl/by-unid/<UNID>/ep0/OTA/Attributes/UIID/<UIID>/LastError/Reported - {"value": "Success"}
ucl/by-unid/<UNID>/ep0/OTA/Attributes/UIID/<UIID>/Status/Reported - {"value": "Idle"}
```

At that point, an IoT Service must interview the node again, and the new
firmware data and capabilites will be available.

![Re-interview button in DevGUI](./doc/assets/img/dev_gui_node_interview_button.png "Re-interview button in DevGUI")

### Possible Errors

There are a few possible errors described in the Unify specification.
The *LastError* attribute indicates the status of the last Firmware
Update/transfer attempt.

![Errors during Firmware Update](./doc/assets/img/dev_gui_possible_firmware_udpate_errors.png "Errors during Firmware Update")

* __InvalidImage__
This error happens if the node rejected the image. Reasons can include the
signature verification, the version downgrade protection or a wrong checksum.
* __NotSupported__
This error happens if the node's firmware is not upgradable. This error will be
reported by the end node before any transfer is attempted.
* __Aborted__
This error can happen if the transfer could not complete without a timeout.

In the case of an abort, you can retry a Firmware Update by modifying the Image
Provider list of images. Remove the UIID(/UNID) combination, wait for the image
Provider to advertise that the image is no longer available, and modify back the
image list with the desired UIID(/UNID) combination.

## Z-Wave Certification Information

### Device Type and Role Type

The ZPC has a Generic Controller Device type and uses the following device classes.
  * GENERIC_TYPE_GENERIC_CONTROLLER
  * SPECIFIC_TYPE_NOT_USED

The ZPC uses the Central Static Controller (CSC) role type, which
supports and takes the SIS role by default.

The ZPC does not support the following network functionalities:
  * Learn Mode
  * Failed Replace

### Manufacturer-Specific Information

The ZPC uses the following manufacturer-specific information:
  * Manufacturer ID : 0x0000
  * Product Type    : 0x0005
  * Product ID      : 0x0001
  * Firmware ID     : 0x0001


### Network Management Information

This product can be operated in any Z-Wave network with other Z-Wave-certified
devices from other manufacturers. All mains operated nodes within the network
will act as repeaters regardless of vendor to increase reliability of the
network.

#### Network Management

The [Dev GUI User's Guide](./md_applications_dev_ui_dev_gui_readme_user.html)
describes how to perform the following operations:
*  Direct range and Network-Wide Inclusion (Add) of other nodes in the network.
*  Direct range and Network-Wide Exclusion (remove) of other nodes (in any network).

Learn mode and Controller replication are not available.

### Association Command Class Information

The ZPC supports the following Association Groups
| Grouping Identifier            | Maximum Number of Associations | Group Name | Usage/Trigger     |
| ------------------------------ | ------------------------------ | ---------- | ----------------- |
| 1                              | 10                             | Lifeline   | Lifeline reports. |


### Multilevel Switch Command Class Control

The ZPC controls the Multilevel Switch Command Class. It is possible to set and
see the current level of a multilevel switch supporting node.

However, the model on which the Unify system and
[Dev GUI](./md_applications_dev_ui_dev_gui_readme_user.html) operate is based on
The Dotdot model.

This means that Multilevel Switch supporting node will be represented with both
an OnOff and a Level functionality. The Level setting can be changed
independently, but will be interpreted in conjunction with the OnOff State.

In short:

* If OnOff is off, the Multilevel Switch value at the end node will
be 0, no matter what the current level shows.

* If OnOff is on, the Multilevel Switch value at the end node will be according
to the level setting.


### Indicator Command Class Information

The ZPC supports identifying itself. The log output will indicate that a blinking
script is invoked. No visible LED is available for this purpose.

```bash
<datetime> <d> [zwave_command_class_indicator] calling blinker on_time:600ms, off_time:200ms, num_cycles:3
```

### Device Reset Locally Command Class Information

The ZPC can be reset to default. The
[Dev GUI User guide](./md_applications_dev_ui_dev_gui_readme_user.html) details
how to request the ZPC to initiate a reset procedure.


### Command Class Information

The following table shows supported and controlled Z-Wave Command Classes by the ZPC.

| Command Class                  | Version | Support | Control | Security Level              | Comment |
| ------------------------------ | ------- | ------- | ------- | --------------------------- | ------- |
| Basic                          |       2 |         |       x | N/A                         |         |
| Binary Switch                  |       2 |         |       x | N/A                         |         |
| Switch Multilevel              |       4 |         |       x | N/A                         | Partial control: <br>1. we do not use start/stop level change.<br>2. we do not support the 0xFF duration |
| Binary Sensor                  |       1 |         |       x | N/A                         | Control Part is auto-generated. |
| Multilevel Sensor              |      11 |         |       x | N/A                         | Partial control: <br>1. Not all scales are supported <br>2. No regular probing is done  |
| Thermostat Mode                |       3 |         |       x | N/A                         | Partial Control: Not all modes can be set |
| Thermostat Setpoint            |       3 |         |       x | N/A                         | Partial Control: <br>1. No discovery of ambiguous types in v1-v2 <br>2. Only a few setpoints can be configured. <br>3. Precision/size fields in the set are determined <br>automatically by the controller.  |
| Association Group Info (AGI)   |       3 |       x |       x | Network Scheme              |         |
| Device Reset Locally           |       1 |       x |       x | Network Scheme              |         |
| Z-Wave Plus Info               |       2 |       x |       x | Unencrypted                 |         |
| Multi Channel                  |       4 |         |       x | N/A                         |         |
| Door Lock                      |       2 |         |       x | N/A                         | Control Part is auto-generated. |
| Supervision                    |       2 |       x |       x | Unencrypted                 |         |
| Notification                   |       3 |         |       x | N/A                         | Partial Control: <br>1. No Push/Pull discovery is done.<br>2. No Pull sensor support. <br>3. Unknown types are not supported. <br>4. No Regular probing is done.  |
| Manufacturer Specific          |       2 |       x |       x | Network Scheme              |         |
| Powerlevel                     |       1 |       x |         | Network Scheme              |         |
| Inclusion Controller           |       1 |       x |       x | Unencrypted                 |         |
| Firmware Update                |       7 |       x |       x | Network Scheme              |         |
| Battery                        |       3 |         |       x | N/A                         | Control Part is auto-generated. |
| Wake Up                        |       3 |         |       x | N/A                         |         |
| Association                    |       2 |       x |       x | Network Scheme              |         |
| Version                        |       3 |       x |       x | Network Scheme              |         |
| Indicator                      |       3 |       x |         | Network Scheme              |         |
| Time                           |       1 |       x |         | Unencrypted                 |         |
| Multi Channel Association      |       3 |       x |       x | Network Scheme              |         |
| Security 0                     |       1 |       x |       x | Unencrypted                 |         |
| Security 2                     |       1 |       x |       x | Unencrypted                 |         |


### SmartStart Information

The ZPC supports including other nodes using SmartStart. Detailed
steps to manipulate the SmartStart list are given in the
[Dev GUI User's Guide](./md_applications_dev_ui_dev_gui_readme_user.html).

### Z-Wave Long Range Support

The ZPC can operate as a Z-Wave Long Range controller, operating both on
Z-Wave and Z-Wave Long Range PHY/MAC.

Steps to include Z-Wave Long Range nodes using the ZPC are described in the
[Dev GUI User's Guide](./md_applications_dev_ui_dev_gui_readme_user.html).

### SmartStart Supported Protocol Detection

The ZPC can perform SmartStart inclusions both for Z-Wave and Z-Wave
Long Range nodes.

The list of supported protocols for a DSK entry is not advertised in
the Unify SmartStart list, which means that the ZPC, in some cases, needs to discover
which protocols are supported/in range for the SmartStart entry.

If the PreferredProtocols in the SmartStart list entry is contains a single value,
the ZPC will only attempt to include the node with the protocol indicated in the
PreferredProtocols, without any discovery.

If PreferredProtocols contains no value, the ZPC will use its default configured
priority list for inclusion protocols.

If PreferredProtocols contains more than one value, the ZPC will use this list to
attempt to include the node.

If the ZPC has several inclusion protocols candidates either from the
PreferredProtocols or based on its configuration, it will first listen for
SmartStart Prime commands to detect which protocols are available
from the node.

In the following use case, the node supports all protocols among two candidates:

\startuml

' Style for the diagram
skinparam classFontColor black
skinparam classFontSize 10
skinparam classFontName Helvetica
skinparam sequenceMessageAlign center
skinparam shadowing false
skinparam ArrowColor #000000
skinparam ParticipantBackgroundColor #FFFFFF
skinparam ParticipantBorderColor #480509
skinparam SequenceLifeLineBorderColor #001111
skinparam SequenceLifeLineBorderThickness 2
skinparam NoteBackgroundColor #FFFFFF
skinparam NoteBorderColor #000000
skinparam ActorBackgroundColor #FFFFFF
skinparam ActorBorderColor #480509

hide footbox
title Inclusion Protocol candidate discovery

' List of participants
participant "SmartStart list" as upvl
participant "ZPC" as zpc
participant "End node" as node

upvl -> zpc: New Entry in the SmartStart list.\n 2 possible inclusion protocols.

rnote over zpc, node: <b>Initiate protocol discovery</b>

rnote left of zpc: known protocols: none

node -> zpc: Z-Wave Long Range\n<b>SmartStart Prime

rnote left of zpc: known protocols:\n1. Z-Wave Long Range

node -> zpc: Z-Wave\n<b>SmartStart Prime

rnote left of zpc: known protocols:\n1. Z-Wave Long Range\n2. Z-Wave

rnote over zpc, node: <b>All candidates received\n<b>Stopping Protocol discovery.

rnote over zpc, node: Initiate SmartStart Inclusion \nat the next SmartStart Inclusion Request.

\enduml

Example, if the node supports only 1 protocol among 2 candidates:

\startuml

' Style for the diagram
skinparam classFontColor black
skinparam classFontSize 10
skinparam classFontName Helvetica
skinparam sequenceMessageAlign center
skinparam shadowing false
skinparam ArrowColor #000000
skinparam ParticipantBackgroundColor #FFFFFF
skinparam ParticipantBorderColor #480509
skinparam SequenceLifeLineBorderColor #001111
skinparam SequenceLifeLineBorderThickness 2
skinparam NoteBackgroundColor #FFFFFF
skinparam NoteBorderColor #000000
skinparam ActorBackgroundColor #FFFFFF
skinparam ActorBorderColor #480509

hide footbox
title Inclusion Protocol candidate discovery

' List of participants
participant "SmartStart list" as upvl
participant "ZPC" as zpc
participant "End node" as node

upvl -> zpc: New Entry in the SmartStart list.\n 2 possible inclusion protocols.

rnote over zpc, node: <b>Initiate protocol discovery</b>

rnote left of zpc: known protocols: none

node -> zpc: Z-Wave Long Range\n<b>SmartStart Prime

rnote left of zpc: known protocols:\n1. Z-Wave Long Range

node -> zpc: Z-Wave Long Range\n<b>SmartStart Prime

rnote left of zpc: known protocols:\n1. Z-Wave Long Range

rnote over zpc, node: <b>Cycle detected for supported protocols\n<b>Stopping Protocol discovery.

rnote over zpc, node: Initiate SmartStart Inclusion \nat the next SmartStart Inclusion Request.

\enduml

The result of this procedure (protocol discovery) will not be saved and will be
performed at every SmartStart inclusion attempt. It allows to re-assess, every
time if a protocol became available. The list of protocols may change due to
firmware update or simply due to radio conditions. Z-Wave nodes may
come within NWI range after including more nodes in the network, and subsequently join themselves.
Therefore, this protocol discovery procedure may happen at every inclusion attempt.

In a Z-Wave network, NWI will be enabled when listening to SmartStart Primes messages
and repeated frames will be received by the ZPC. To prevent
repeated frames from triggering an early conclusion on which protocols are supported, the
ZPC will apply a back-off after receiving a prime message. The back-off should be
larger than two seconds.

\startuml

' Style for the diagram
skinparam classFontColor black
skinparam classFontSize 10
skinparam classFontName Helvetica
skinparam sequenceMessageAlign center
skinparam shadowing false
skinparam ArrowColor #000000
skinparam ParticipantBackgroundColor #FFFFFF
skinparam ParticipantBorderColor #480509
skinparam SequenceLifeLineBorderColor #001111
skinparam SequenceLifeLineBorderThickness 2
skinparam NoteBackgroundColor #FFFFFF
skinparam NoteBorderColor #000000
skinparam ActorBackgroundColor #FFFFFF
skinparam ActorBorderColor #480509

!pragma teoz true

hide footbox
title Inclusion Protocol with NWI repeaters

' List of participants
participant "SmartStart list" as upvl
participant "ZPC" as zpc
participant "NWI Repeater" as repeater
participant "End node" as node

upvl -> zpc: New Entry in the SmartStart list.\n 2 possible inclusion protocols.

rnote over zpc, node: <b>Initiate protocol discovery</b>

rnote left of zpc: known protocols: none

node -> zpc: Z-Wave\n<b>SmartStart Prime
&node -> repeater

rnote left of zpc: known protocols:\n1. Z-Wave
rnote left of zpc: Start back-off for Z-Wave
activate zpc

repeater -> zpc: Z-Wave\n<b>SmartStart Prime
rnote left of zpc: Ignoring repeater\nmessage in back-off


zpc -> zpc: end of back-off for Z-Wave
deactivate zpc

node -> zpc: Z-Wave\n<b>SmartStart Prime

rnote left of zpc: known protocols:\n1. Z-Wave
rnote over zpc, node: <b>Cycle detected for supported protocols\n<b>Stopping Protocol discovery.

rnote over zpc, node: Initiate SmartStart Inclusion \nat the next SmartStart Inclusion Request.

\enduml

## Transitioning from Z/IP Gateway + Z-Ware

The ZPC provides a subset of Z-Wave features compared to the Z/IP Gateway.

* The ZPC is based on Generic Controller Device type and it does not support Learn Mode that enables
  it to be included to other networks.
* The ZPC does not contain Installation and Maintenance Framework that provides a method for gathering
  statistics and performing network maintenance.
* The ZPC does not allow to establish arbitrary Associations and only establishes Lifeline Association.
* The ZPC does not control all Command Classes fully. For some, the ZPC only provides partial control.
  Z-Wave Command Classes where the ZPC provides partial control include:
  + Switch Multilevel, version 4,
  + Multilevel Sensor, version 11,
  + Thermostat Mode, version 3,
  + Thermostat Setpoint, version 3 and
  + Notification, version 11.