# ZPC Release Notes

This document describes the changes between the last released ZPC and the current release.

## [1.1.0] - Feb 2022

### Added

* Full Control for Central Scene Command Class
  * Populating Scene Attributes to reflect the node's capabilities (total number of scenes)
  * Central Scene is mapped to the Scene Cluster.

* Issuing Generated Commands based on interaction with Z-Wave nodes
  * Issuing RecallScene when receiving Central Scene Notifications.
  * Issuing RF Telemetry reports at each transmission when RF Telemetry is enabled

* Support for RF Telemetry (reporting transmissions events)
* Monitoring if Offline nodes are responding again with regular NOPs commands.
* Network Polling

### Changed

* ZPC datastore file key has been changed from --datastore.file to --zpc.datastore_file
* Default datastore file has been changed from /var/lib/uic/database.db to /var/lib/uic/zpc.db
  * If you used the default configuration, keep your previous database by running `mv /var/lib/uic/database.db /var/lib/uic/zpc.db`

* Increased Z-Wave Tx Queue robustness to multiple Tx sessions.
* Communication with Z-Wave Module optimized

### Fixed

* Interviewing an offline node would make it appear online.
* Missing Wake Up Interval Set during interview for door/window sensor
* ZPC hangs and stops resolving attributes when DevGUI sends a MoveWithOnOff command

## [1.0.3] - Nov 2021

## [1.0.2] - Oct 2021

### Added

* ZPC is Z-Wave Certified against the Z-Wave Specification release 2021A.
* Support of Z-Wave Command Classes
  * Firmware Update
* Control of Z-Wave Command Classes
  * Firmware Update
  * Indicator

### Changed

* OTA UIID has been updated to include hardware revision

### Fixed

* Fixed various certification issues

## [1.0.1] - Aug 2021

### Fixed

* Build issues

## [1.0.0] - Aug 2021

### Added

* Implementation of Unify specification 1.0
* Support for performing Over The Air update(OTA) of Z-Wave devices
* Z-Wave Long Range
* MQTT TLS encryption support
* Wake up device support
* Multi cast support
* Failed node handling
* Support for Z-Wave command classes
  * Indicator
  * Version
  * Manufacturer
  * Z-Wave plus
  * PowerLevel
* Control of Z-Wave command classes
  * Alarm
  * Firmware Update
  * Binary Sensor
  * Inclusion controller
  * Association Group Information
  * Association
  * Battery
  * Binary Switch
  * Wake up
  * Door lock
* Z-Wave support for Z-Wave transports
  * Multi command
  * Multicast non-secure and S2
  * Transport Service
  * Multi Channel
  * Security0
  * Security2

* ZCL Mappings
  * IAZ FireSensor
  * IAZ COSensor
  * IAZ WaterSensor
  * IAZ BatteryLow
  * TemperatureMeasurement
  * Basic
  * DoorLock
  * Level
  * IlluminanceLevelSensing
  * OccupancySensing
  * OnOff

### Changed

* Various bug fixes
* UAM Language updates
* Documentation updates

# Known Bugs

| ID        | Known issues                                                                                          | Note                                            |
|-----------|-------------------------------------------------------------------------------------------------------|-------------------------------------------------|
| UIC-1162  | When device have multiple functionalities on the Z-Wave end, which is mapped to the same ZigBee attribute, the mapping will be incomplete.   | Update the UAM file to avoid overlap.           |
| UIC-1088  | Potentially wrong controller NIF after controller migration.                                          | Usually no problem in practice                  |
| UIC-1603  | When ZPC includes a sleeping device, sometimes the node interview does not succeed and therefore, the full functionality of the node will not be presented to the use. | Trigger a re-interview of the node to resolve the issue. |
| UIC-1650  | Very rare race condition gets the ZPC stuck during a reset operation. | Restart the ZPC if it did not respawn 20 seconds after reset |

# Z-Wave Certification information

See [ZPC Certification Guide](readme_certification.md)

# Devices tested with the ZPC

Devices which are tested with the ZPC can be found in the list below. Working
devices will be marked with an **x** and none working devices will be marked
with an **-**.

| Vendor           | Name                          | Working (x) or not working (-) | Security inclusion |
| ---------------- | ----------------------------- | ------------------------------ | ------------------ |
| Aeotec           | LED Bulb Multi-White          | x                              | S0                 |
| Aeotec           | Door/Window Sensor            | x                              | S0                 |
| Aeotec           | Water sensor (6 GEN)          | -                              | Non-Secure         |
| Aeotec           | Range extender 6              | x                              | S0                 |
| Aeotec           | Door/Window sensor 6          | x                              | S0                 |
| Hank             | Door/Window Sensor            | x                              | S2-Unauthenticated |
| Hank             | RGB Bulb                      | x                              | Non-Secure         |
| Hank             | Smart plug                    | x                              | S0                 |
| Fibaro           | Fibaro Flood Sensor           | x                              | S0                 |
| Fibaro           | Heat Controller               | x                              | S2-Authenticated   |
| Philio           | Smart energy plug-in switch   | x                              | S0                 |
| Secure           | Smart Plug 302                | x                              | Non-Secure         |
| Ring             | Alarm motion detector (1 Gen) | -                              | S2-Authenticated   |
| Yale real living | Door Lock                     | x                              | S0                 |
| Alarm.com        | Smart water valve             | x                              | S2 Authenticated   |
| Alarm.com        | Dry contact sensor            | -                              | Non-Secure         |
