# ZigPC (Beta) Release Notes
This document describes the changes between the current and last released ZigPC.

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
| Building in Docker may fail due to Xvfb failing.                                                                                                      | Run `Xvfb -ac ${DISPLAY} -screen 0 640x480x8 -nolisten tcp > /dev/null 2>&1 &` in Docker.   |
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

### Zigbee Devices Used for Testing
| Vendor    | Name                                   | Device Type         | Working | Notes                                                                                                                                                          |
| --------- | ---------------------------------------| ------------------- | ------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Silabs    | xG12 or xG21 radio (NCP w/ EZSP-UART)  | <br/>               | Yes     | <br/>                                                                                                                                                          |
| Silabs    | xG12 or xG21 radio                     | Z3Light             | Yes     | <br/>                                                                                                                                                          |
| Silabs    | xG12 or xG21 radio                     | Z3Switch            | Yes     | Since Z3Switch only has a OnOff client cluster, attribute reports and commands are not supported for this cluster.                                             |

# Zigbee Certification information
> _NOTE the ZigPC is at this point not certifiable_