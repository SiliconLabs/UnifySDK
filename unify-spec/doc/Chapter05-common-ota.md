# Common OTA FW Update Service

This chapter describes a common firmware update mechanism for all wireless
technologies supported by the Unify framework. The design goal of this framework
is to provide a common interface for applying firmware images to Unify end
nodes.

The overall design is that there is an OTA Image Provider, which has a database of firmware images and that
each protocol controller implements a firmware consumer on behalf of its PAN nodes. The Image consumer monitors the images provided by the image provider and compares the firmware versions of the running devices. If a mismatch of versions is identified, the consumer will poll the image file from the consumer and transfer the image to the PAN device using a protocol-specific OTA method.


\startuml
database ImageProvider
node MQTT
node ProtocolController {
  node ImageConsumer
}
node PanDevice

ImageProvider --> MQTT
MQTT --> ImageConsumer
ProtocolController--> PanDevice
\enduml

## UCL OTA Framework

The UCL OTA framework supports updating the firmware PAN devices connected to the MQTT broker through protocol
controllers. Each PAN device can have any number of firmware images needed to operate the device. Examples of firmware images are Protocol Stack code, Application code, and a bitmap image used as background in a display. The framework supports updating both individual nodes and all nodes, which may support a given firmware image.

Because each image can be anything, the format of the firmware data file is not mandated by this specification. Examples of firmware file data are the following:
 - Gecko Boot loader(GBL)
 - Z-Wave 500 series boot loader files
 - PNG image
 - others...

This framework does not in itself provide a way to verify the integrity of firmware images. Each
protocol controller / end node needs to ensure that the image verification takes place, i.e., allowing only signed firmware images to be activated in the end device.

It is assumed that the firmware provider MQTT client is authenticated with the MQTT broker. The Unify Access Control
system must prevent unauthenticated firmware providers from publishing on the OTA namespaces (see below).

Each firmware image is identified by a Unique Image Identifier (UIID). The UIID is a literal string that messages where this firmware image belongs. In other words, this is the application firmware of manufacturer A product B and hardware rev C. The UIID identifies a specific image _destination_ but it does not identify the image version. In other words, version 1.00 and version 1.12 of the firmware image of a particular light bulb have the same UIID.

The table below shows suggestions how to generate different UIID strings.

> This specification only dictates that the UIID should be prefixed with a protocol-specific code.
> How the string is constructed is protocol specific.

Technology    | UIID components                                  | Suggestion
------------- | ------------------------------------------------ | -------------
ZigBee        |  "ZigBee"+ManufacturerCode+ImageType             | ZigBee010103
Z-Wave        |  "ZWave"+ManufacturerId+ProductID+FirmwareTarget | ZWave0001000100
Bluetooth     |  "BT"+ CompanyID+VendorInfo                      | TBD


## UCL OTA Communication Protocol

The OTA communication protocol is between two types of components, _image providers_ and _image consumers_.
The image provider is responsible for keeping a repository of firmware images and announce which images are available.
The image provider also manages MQTT topics in the ucl/OTA namespace. This is an independent MQTT client. The image provider operates with two topic namespaces. One namespace is used to apply firmware images to a specific device, while the other namespace is for applying firmware one firmware image to all devices which support the image.
The image consumers manages topics in the ucl/by-unid/\<UNID\>/OTA/\<UIID\> namespace, which provide information about the firmware download progress.

The following MQTT topics are relevant for the firmware update process:

Topic                               | Image Providers       | Image Consumers       | Description
------------------------------------|-----------------------|-----------------------|-------------
ucl/OTA/info/\<UIID\>/all           | Publish(retained)      | Subscribe             | Meta info for general images
ucl/OTA/info/\<UIID\>/\<UNID\>      | Publish(retained)      | Subscribe             | Meta info for an image to be pushed to a particular device.
ucl/OTA/data/\<UIID\>/all/get       | Subscribe              | Publish(un-retained ) | Trigger data publish for general images
ucl/OTA/data/\<UIID\>/\<UNID\>/get  | Subscribe              | Publish(un-retained ) | Trigger data publish to a particular device.
ucl/OTA/data/\<UIID\>/all           | Publish(un-retained )  | Subscribe             | Data for general images
ucl/OTA/data/\<UIID\>/\<UNID\>      | Publish(un-retained )  | Subscribe             | Data for an image to be pushed to a particular device.
ucl/by-unid/\<UNID\>/ep\<ID\>/OTA/Attributes/UIID/\<UIID\>/# | N/A                   | Publish(retained)     | Status of firmware update of a particular device.
ucl/by-unid/\<UNID\>/ep\<ID\>/OTA/Attributes/ClusterRevision | N/A                   | Publish(retained)     | OTA Cluster revision
ucl/by-unid/\<UNID\>/ep\<ID\>/OTA/SupportedCommands | N/A                   | Publish(retained)     | List of commands that can be executed for a device.

### Announcing Images

When an image is available, the Image Provider publishes meta information about available images on the general topic
ucl/OTA/info/\<UIID\>/all or optionally on the ucl/OTA/info/\<UIID\>/\<UNID\>. The latter topic is device specific and can be used to provide an image for a specific device. The two firmware info messages have the same format, but the device-specific image has precedence over the general topic.

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "OTA Image Info",
  "description": "Information about a firmware image",
  "type": "object",
  "properties": {
    "Version": {
      "type": "string"
    },
    "ApplyAfter": {
      "type": "string",
      "format": "date-time"
    },
    "Filename": {
      "type": "string"
    }
  },
  "required": [
    "Version"
  ]
}
```
There a two fields in the firmware info object:
- _Version_ A string that identifies the version of the firmware image. If the a PAN node is running a lower
  version of the firmware, it should apply the image. Comparing image versions is not defined by this
  specification because this may be protocol controller specific.
- _ApplyAfter_ This is the minimum timestamp when to apply this image. A possible use
  case is as follows:
  _I want to upgrade all my light bulbs at 1AM as everyone is sleeping_.
  A protocol controller may start to transfer the image before this
  timestamp, if the device does not activate the image by itself.
  This value MUST comply with the RFC 3339.
  If this parameter is omitted, the Firmware Update should take place as soon
  as possible.

### Retrieving Images

Protocol controllers must subscribe to the ucl/OTA/info/+ topic to monitor the availability of new firmware images.
When a protocol controller identifies that a new image is available, it must request the image data from the
image provider. It does so by temporarily subscribing to ucl/OTA/data\<UIID\>/all or ucl/OTA/data\<UIID\>/\<UNID\>, and then publishing an empty JSON object {} message on ucl/OTA/data\<UIID\>/all/get or ucl/OTA/data\<UIID\>/\<UNID\>/get. This MUST trigger the image provider to do a publish of the entire image on the ucl/OTA/data/\<UIID\>/all or ucl/OTA/data\<UIID\>/\<UNID\> topics.
If the image provider does not recognize the UIID, the get command must be ignored.
The publish of the _data_ topic MUST not be retained. MQTT allows for a maximum payload of 256 MB which should be enough for the firmware image.

After getting the firmware image, the protocol controller must start updating the devices affected. If the protocol controller needs to send the same image to multiple devices, the protocol controller SHOULD cache the
image and not request the same image multiple times.

### Providing Firmware Status

Protocol controllers must announce the current firmware status for all devices and all active firmware images. The
protocol controller publishes retained on ucl/by-unid/\<UNID\>/OTA/\<UIID\>/+ topic pattern. The firmware update status is used by IOT services to monitor the firmware download progress. If a device firmware update leads to
an UIID no longer there, the protocol controller must un-retain all attributes on the given UIID. For example, _the display background in now contained in the firmware file_.

The following attributes are defined as regular DotDot attributes.
An endpoint prefix is also used. If the Firmware Update functionality applies
for the entire device, it should be reported under endpoint 0.

<table>
<caption>SmartStart parameters</caption>
<tr><th>Topic</th> <th>JSON data type</th></tr>
<tr>
  <td>
    ucl/by-unid/\<UNID\>/ep\<ID\>/OTA/Attributes/UIID/\<UIID\>/CurrentVersion/Desired<br>
    ucl/by-unid/\<UNID\>/ep\<ID\>/OTA/Attributes/UIID/\<UIID\>/CurrentVersion/Reported
  </td>
  <td>String. Example: {"value": "0x070F01"}</td>
</tr>

<tr>
  <td>
    ucl/by-unid/\<UNID\>/ep\<ID\>/OTA/Attributes/UIID/\<UIID\>/TargetVersion/Desired<br>
    ucl/by-unid/\<UNID\>/ep\<ID\>/OTA/Attributes/UIID/\<UIID\>/TargetVersion/Reported
  </td>
  <td>String. Example: {"value": "0x070F02"}</td>
</tr>

<tr>
  <td>
    ucl/by-unid/\<UNID\>/ep\<ID\>/OTA/Attributes/UIID/\<UIID\>/ApplyAfter/Desired<br>
    ucl/by-unid/\<UNID\>/ep\<ID\>/OTA/Attributes/UIID/\<UIID\>/ApplyAfter/Reported
  </td>
  <td>Data-time formatted String. Example: {"value": "2021-04-15T23:59:59+00:00"} </td>
</tr>

<tr>
  <td>
    ucl/by-unid/\<UNID\>/ep\<ID\>/OTA/Attributes/UIID/\<UIID\>/Status/Desired<br>
    ucl/by-unid/\<UNID\>/ep\<ID\>/OTA/Attributes/UIID/\<UIID\>/Status/Reported
  </td>
  <td>String with enumeration. Possible values are :<br>
  ["Idle","DownloadInProgress","WaitingToUpgrade",<br>
  ,"WaitForMore","WaitingToUpgradeViaExternalEvent"]<br>
  Example: {"value": "DownloadInProgress"}  </td>
</tr>

<tr>
  <td>
    ucl/by-unid/\<UNID\>/ep\<ID\>/OTA/Attributes/UIID/\<UIID\>/Size/Desired<br>
    ucl/by-unid/\<UNID\>/ep\<ID\>/OTA/Attributes/UIID/\<UIID\>/Size/Reported
  </td>
  <td>Integer value. Example {"value": 80000}</td>
</tr>

<tr>
  <td>
    ucl/by-unid/\<UNID\>/ep\<ID\>/OTA/Attributes/UIID/\<UIID\>/Offset/Desired<br>
    ucl/by-unid/\<UNID\>/ep\<ID\>/OTA/Attributes/UIID/\<UIID\>/Offset/Reported
  </td>
  <td>Integer value. Example {"value": 1250}</td>
</tr>

<tr>
  <td>
    ucl/by-unid/\<UNID\>/ep\<ID\>/OTA/Attributes/UIID/\<UIID\>/LastError/Desired<br>
    ucl/by-unid/\<UNID\>/ep\<ID\>/OTA/Attributes/UIID/\<UIID\>/LastError/Reported
  </td>
  <td>String with enumeration. Possible values are :<br>
  ["Success","Abort","NotAuthorized","InvalidImage","RequireMoreImage","NotSupported"] <br>
  Example: {"value": "Success"}  </td>
  </td>
</tr>
<tr>
  <td>
    ucl/by-unid/\<UNID\>/ep\<ID\>/OTA/Attributes/ClusterRevision/Desired<br>
    ucl/by-unid/\<UNID\>/ep\<ID\>/OTA/Attributes/ClusterRevision/Reported
  </td>
  <td>Integer value. This value MUST be set to 1 for this Cluster. Future Unify
  Specifications may add functionalities to this cluster and increment
  this value. <br>
  JSON payload Example : {"value": 1} <br>
  </td>
</tr>
</table>

The descriptions of the attributes are the following:

- _CurrentVersion_ The current version of the running image
- _TargetVersion_  The version which will be installed. If this is identical to CurrentVersion, the Status must be _Normal_)
- _ApplyAfter_     Timestamp for when to start the firmware transfer, which is obtained form the image provider via the
  info topic
- _Status_          Status of the transfer.
  | Status              | Description
  |---------------------|-------------------------------------------------------------------------------------------------
  | Idle                | The device is not currently participating in any part of an OTA process.
  | DownloadInProgress  | One ore more firmware fragments has been sent but the transfer is not complete.
  | WaitingToUpgrade    | Indicates that device has transferred the image but has not yet updated. Possibly because of the ApplyAfter timestamp has not yet passed.
  | WaitingToUpgradeViaExternalEvent | Device needs external/manual activation to complete the update.
  | WaitForMore         | Indicates that the client is still waiting to receive more OTA upgrade image files from the server. This is true for a client device that is composed of multiple processors and each processor requires different image. The client SHALL be in this state until it has received all necessary OTA upgrade images, then it SHALL transition to Download complete state.

- _Size_            Size of the image which is to be transferred in bytes.
- _Offset_          Offset of the next chunk to transfer.
- _LastError_       Error code, if any, of the last operation, the following codes are available
  | LastError           | Description
  |---------------------|------------------
  | SUCCESS             | The operation was successful.
  | ABORT               | Failed case when a client or a server decides to abort the upgrade process.
  | NOT_AUTHORIZED      | The Node expected some additional authorization (e.g. out of band activation) to accept updating its firmware.
  | INVALID_IMAGE       | Invalid OTA upgrade image (ex. failed signature validation or signer information check or CRC check)
  | REQUIRE_MORE_IMAGE  | The client still requires more OTA upgrade image files in order to successfully upgrade
  | NOT_SUPPORTED       | The node does not support upgrading this firmware.
- _ClusterRevision_  The Cluster Revision for the generic OTA cluster.


## Z-Wave Firmware Update Example

The ZPC (Z-Wave Protocol Controller) MUST provide the /OTA topic for all Z-Wave
nodes supporting the Firmware Update Command Class.

It must gather a list of UIID for all images using information from the
Version Command Class as well as information from the Firmware Update Command
Class and subscribe the ucl/OTA/info/\<UIID\>/all and
ucl/OTA/info\<UIID\>/\<UNID\> topics for all its nodes
(or uses a wildcard and filters on incoming messages).

The version of the announced images are compared to the version of the
running images. If a version mismatch is found, the ZPC requests the
image by issuing a /OTA/data/.../get publish.

When the image is received on the /OTA/data/... topic, the ZPC must calculate
the CRC16 checksum of the image before starting the transfer the data to the
Z-Wave node.
The ucl/by-unid/\<UNID\>/ep\<ID\>/OTA/Attributes/UIID/\<UIID\> topic is updated by the ZPC
as the firmware transfer and update progresses.

The Z-Wave node will start requesting firmware fragments.
Because the Z-Wave node is requesting fragments, the ZPC MUST update the
"offset" attribute.

The firmware transfer is done on the Z-Wave side after receiving a
Firmware Update Firmware Update Meta Data Status Report Command.
If the Z-Wave node sends the command with the status code set to 0xFD
(wait for activation), the ZPC MUST wait to the time noted in the apply_after
attribute to send the Firmware Update Activation Set Command to the Z-Wave node.

If the device supports activation, the image will be applied immediately
and the UUID status attribute must be update immediately.

If the node does not support delayed activations, the ZPC SHOULD indicate it.

### Z-Wave OTA Example with Activation Support

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

' Allows to do simultaneous transmissions
!pragma teoz true

title Z-Wave OTA update with activation

legend top
<font color=#0039FB>MQTT Subscription</font>
<font color=#00003C>Retained MQTT Publication</font>
<font color=#6C2A0D>Unretained MQTT Publication</font>
endlegend

participant "Image Provider" as IS
participant "MQTT Broker" as MQTT
participant "Z-Wave Protocol Controller" as PC
participant "Z-Wave Node" as Node

IS -> MQTT: <font color=#0039FB>ucl/ota/data/<UIID>/all/get
IS -> MQTT: <font color=#0039FB>ucl/ota/data/<UIID>/+/get

PC -> MQTT: <font color=#0039FB>ucl/OTA/info/+/all
PC -> MQTT: <font color=#0039FB> ucl/ota/data/<UIID>/all

note over PC, Node: Z-Wave Firmware Update \nand Version interview
note over IS, Node: Skipping Desired attributes\npublications in this diagram for simplicity.

PC -> MQTT: <font color=#00003C>ucl/by-unid/<UNID>/ep0/OTA/Attributes/ClusterRevision/Reported \n{"value":1}
PC -> MQTT: <font color=#00003C>ucl/by-unid/<UNID>/ep0/OTA/Attributes/UIID/<UIID>/CurrentVersion/Reported \n{"value":"0x070F01"}
PC -> MQTT: <font color=#00003C>ucl/by-unid/<UNID>/ep0/OTA/Attributes/UIID/<UIID>/TargetVersion/Reported \n{"value":"0x070F01"}
PC -> MQTT: <font color=#00003C>ucl/by-unid/<UNID>/ep0/OTA/Attributes/UIID/<UIID>/ApplyAfter/Reported \n{"value":"2020-12-12T03:29:09+00:00"}
PC -> MQTT: <font color=#00003C>ucl/by-unid/<UNID>/ep0/OTA/Attributes/UIID/<UIID>/Status/Reported \n{"value" : "Idle"}
PC -> MQTT: <font color=#00003C>ucl/by-unid/<UNID>/ep0/OTA/Attributes/UIID/<UIID>/Size/Reported \n{"value": 0}
PC -> MQTT: <font color=#00003C>ucl/by-unid/<UNID>/ep0/OTA/Attributes/UIID/<UIID>/Offset/Reported \n{"value": 0}
PC -> MQTT: <font color=#00003C>ucl/by-unid/<UNID>/ep0/OTA/Attributes/UIID/<UIID>/LastError/Reported \n{"value": "Success"}

note right of IS: New image is available to all
IS -> MQTT: <font color=#00003C>ucl/OTA/info/<UIID>/all \n\t{"Version":"0x070F03", \n\t"ApplyAfter":"2021-04-15T23:59:59+00:00", \n\t"Filename":"ZW_PowerStrip_7.16.0_90_ZGM130S_REGION_US_LR.gbl"}
&MQTT->PC:

note over PC: UIID is matching for Z-Wave Node UNID.\nPull the image.
PC -> MQTT: <font color=#6C2A0D>ucl/ota/data/<UIID>/all/get {}
&MQTT->IS:
IS -> MQTT: <font color=#6C2A0D>ucl/ota/data/<UIID>/all \n//binary data//
&MQTT->PC:

note right of PC: Accepts the new data\nStart the Firmware Update procedure
PC -> MQTT: <font color=#00003C>ucl/by-unid/<UNID>/ep0/OTA/Attributes/UIID/<UIID>/TargetVersion/Reported \n{"value":"0x070F03"}
PC -> MQTT: <font color=#00003C>ucl/by-unid/<UNID>/ep0/OTA/Attributes/UIID/<UIID>/ApplyAfter/Reported \n{"value":"2021-04-15T23:59:59+00:00"}

PC -> Node: Firmware Meta Data Request Get\n(Firmware ID value based on UIID)
Node -> PC: Firmware Meta Data Request Report\n(Status = OK)

PC -> MQTT: <font color=#00003C>ucl/by-unid/<UNID>/ep0/OTA/Attributes/UIID/<UIID>/Status/Reported \n{"value":"DownloadInProgress"}
PC -> MQTT: <font color=#00003C>ucl/by-unid/<UNID>/ep0/OTA/Attributes/UIID/<UIID>/Size/Reported \n{"value": 100000}
PC -> MQTT: <font color=#00003C>ucl/by-unid/<UNID>/ep0/OTA/Attributes/UIID/<UIID>/Offset/Reported \n{"value":0}

loop For each firmware fragment i = 1..X
  Node -> PC: Firmware Update Meta Data Get\n(Report Number = i)
  PC -> MQTT: <font color=#00003C>ucl/by-unid/<UNID>/ep0/OTA/Attributes/UIID/<UIID>/Offset/Reported \n{"value": i*fragment_size}
  PC -> Node: Firmware Update Meta Data Report\n(Report Number = i, data)
end

Node -> PC: Firmware Update Meta Data Status Report\n(Status = 0xFD (wait for activation)
PC -> MQTT: <font color=#00003C>ucl/by-unid/<UNID>/ep0/OTA/Attributes/UIID/<UIID>/Status/Reported \n{"value":"WaitingToUpgrade"}

...
note over PC: Timestamp for activation passes

PC -> Node: Firmware Update Activation Set\n(Firmware ID value based on UIID)
Node -> PC: Firmware Update Activation Status Report\n(Status = Compeleted, Wait Time = 30s)
PC -> MQTT: <font color=#00003C>ucl/by-unid/<UNID>/ep0/OTA/Attributes/UIID/<UIID>/LastError/Reported \n{"value": "Success"}
note over Node: reboots and applies firmware
Node -> PC: Firmware Update Activation Status Report\n(Status = Compeleted, Wait Time = 0s)
PC -> MQTT: <font color=#00003C>ucl/by-unid/<UNID>/ep0/OTA/Attributes/UIID/<UIID>/LastError/Reported \n{"value": "Success"}
PC -> MQTT: <font color=#00003C>ucl/by-unid/<UNID>/ep0/OTA/Attributes/UIID/<UIID>/Status/Reported \n{"value":"Idle"}
PC -> MQTT: <font color=#00003C>ucl/by-unid/<UNID>/ep0/OTA/Attributes/UIID/<UIID>/CurrentVersion/Reported \n{"value":"0x070F03"}

\enduml
