# ZigPC Release Notes
This document describes the changes between the current and last released ZigPC.

## [1.1.0] - Feb 2022

### Added
* Debian Installer also configures ZigPC to run as a system service
* Ability to get ZigPC specific diagnostic information (uptime, CPU use, memory
use, Zigbee stack counters) from the host and the coordinator
* ZAP and SLC_CLI has been integrated as project dependencies for Zigbee host
library generation
* Attribute polling if report configuration fails
* Publishing GeneratedCommands sent from Zigbee devices as responses or as
unsolicited events

### Changed
* Migrate Zigbee-Host library (EmberZNet-based) from Appbuilder project
  (GeckoSDK 3.1) to SLC project (GeckoSDK 4.0). See
  [Zigbee Host Library Readme](components/zigpc_gateway/libs/zigbee_host/readme.libzigbee_host.zigpc.md)
  for more details. NOTE: In order to listen to new ZCL cluster messages, the
  SLC project configuration should be changed in addition to the ZAP dotdot helpers
* ZigPC datastore file key has been changed from --datastore.file to --zigpc.datastore_file
* Default datastore file has been changed from /var/lib/uic/database.db to /var/lib/uic/zigpc.db
If you used the default configuration, keep your previous database by running `mv /var/lib/uic/database.db /var/lib/uic/zigpc.db`

### Removed
* Use of EmafDeviceTablePlugin in Zigbee-Host library to manage device discovery;
This is now performed within ZigPC

### Fixed
* Persisted information cleanup on device leaving the network
* JSON payloads accepted for commands and attributes to be UCL compliant


## [1.0.3] - Nov 2021

### Added
* Add persistence of Zigbee device discovery information to allow servicing
devices after ZigPC reboots
* Use of NCP address-table to manage EUI64 to NodeId mappings
### Changed
* Updates to network management
* Improvements in Zigbee Host EZSP message performance
* Merge ZigPC node state and ucl handling components
* Reduce dependence on device table plugin, use address table
### Fixed
* Improved ZCL frame error handling and empty string bug fix
* Node unretain messages bugfix
* Empty group name bugfix
* Added polling for unreportable attributes

## [1.0.2] - Oct 2021

## [1.0.1] - Aug 2021
### Fixed
* Build issues

## [1.0.0] - Aug 2021
### Added
* Implementation of Unify specification 1.0
* Sleepy end device support
* Support for performing Over The Air update(OTA) of Zigbee devices
* Multicast/Groups support
* Zigbee Trust Center support
* Zigbee device provisioning using Z3 Install Codes
* Attribute reporting configuration on supported clusters
* UCL ForceReadAttribute support
* UCL to ZCL command + attribute mappings
  + (0x0003) Identify
  + (0x0004) Groups
  + (0x0006) OnOff
  + (0x0008) Level
  + (0x0101) DoorLock
  + (0x0201) Thermostat
  + (0x0300) ColorControl
  + (0x0406) OccupancySensing
* Zigbee Host-NCP EZSP-UART Support
### Changed
* Various bug fixes
* Documentation updates

# Known Issues
| Known Issue                                                                                                                                           | Workaround
|-------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------|
| If connection to the MQTT broker is lost, ZigPC does not re-subscribe to topics                                                                       | Restart ZigPC.                                                                              |
| User may experience outdated devices that are still listed from cached data in the browser                                                            | A fresh browser session can remove outdated devices.                                        |
| Network management state is not published correctly if UPVL is started after ZigPC                                                                    | Start UPVL before ZigPC.                                                                    |
| The UPVL becomes unresponsive to MQTT messages and will miss MQTT messages published to "ucl/SmartStart/List/Update"                                  | Restart UPVL.                                                                               |
| Including a node using the SmartStart list is only effective for 18 hours                                                                             | Ensure that a node is added within 18 hours of being put in the SmartStart List.            |
| When performing network-steering, the device will either fail node addition or node interview partially.                                              | In the device's EmberCLI, enter the commands `network leave` and `keys clear` then retry.   |
| When excluding devices, the protocol UNID is not removed from the node's SmartStart entry. This will cause the ZigPC to refuse to re-include the node.| Clear the ZigPC UNID from the SmartStart entry using the Dev UI.                            |
| After completing node removal, the node state information is not cleared from the MQTT topics previously used to service the node.                    |                                                                                             |
| ZigPC does not set the desired attribute values for the DotDot clusters serviced.                                                                     |                                                                                             |
| ZigPC is able to service the DotDot Color Control cluster (0x0300) but not publish any attribute changes.                                             |                                                                                             |
| ZigPC does not persist any network information. devices previously connected to the ZigPC network will not be reachable after ZigPC restarts.         | Ensure the devices have left the ZigPC Zigbee network after ZigPC restarts.                 |
| ZigPC only supports "Remove" and "Idle" PC state change requests via the Node List view in DevUI. "Add" and "Reset" options are not supported.        | To add nodes to the Zigbee network, using the Smartstart DSK process outlined in the readme.|
| ZigPC can fail at initialization if the NCP image is based on GSDK 4.0 without ADDRESS_TABLE being set to 250                                         | Set macro `EMBER_ADDRESS_TABLE_SIZE=250` in NCP studio project before building NCP image.   |
| When using Gecko SDK 4.0, OTA updates using the image provider may be unstable                                                                        | Use the Gecko SDK 3.X to generate OTA files                                                 |

### Zigbee Devices Used for Testing
| Vendor    | Name                                   | Device Type         | Working | Notes                                                                                                                                                          |
| --------- | ---------------------------------------| ------------------- | ------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Silabs    | xG12 or xG21 radio (NCP w/ EZSP-UART)  | <br/>               | Yes     | <br/>                                                                                                                                                          |
| Silabs    | xG12 or xG21 radio                     | Z3Light             | Yes     | <br/>                                                                                                                                                          |
| Silabs    | xG12 or xG21 radio                     | Z3Switch            | Yes     | <br/>                                                                                                                                                          |
| Silabs    | xG12 or xG21 radio                     | Z3Thermostat        | Yes     | <br/>                                                                                                                                                          |
| Silabs    | xG12 or xG21 radio                     | Z3DoorLock          | Yes     | <br/>                                                                                                                                                          |

# Zigbee Certification information
> _NOTE the ZigPC is at this point not certifiable_
