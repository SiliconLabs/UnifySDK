# ZPC Release Notes
This document describes the changes between the last released ZPC and the current release.

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
  + Indicator
  + Version
  + Manufacturer
  + Z-Wave plus
  + PowerLevel
* Control of Z-Wave command classes
  + Alarm
  + Firmware Update
  + Binary Sensor
  + Inclusion controller
  + Association Group Information
  + Association
  + Battery 
  + Binary Switch
  + Wake up
  + Door lock
* Z-Wave support for Z-Wave transports
  + Multi command
  + Multicast non-secure and S2
  + Transport Service
  + Multi Channel
  + Security0
  + Security2

* ZCL Mappings
  + IAZ FireSensor
  + IAZ COSensor
  + IAZ WaterSensor
  + IAZ BatteryLow
  + TemperatureMeasurement
  + Basic
  + DoorLock
  + Level
  + IlluminanceLevelSensing
  + OccupancySensing
  + OnOff
### Changed
  *  Various bug fixes
  *  UAM Language updates
  *  Documentation updates
# Known Bugs
| ID        | Known issues                                                                                          | Note                                            |
|-----------|-------------------------------------------------------------------------------------------------------|-------------------------------------------------|
| UIC-1154	| Upon node removal the ucl/by-unid/UNID/OTA topic may not be cleared.                                  |                                                 |
| UIC-1142	| Debian package overwrites the existing UIC configuration.                                             | Back up the config file before doing the update.|
| UIC-1162  | When device have multiple functionalities on the Z-Wave end,which is mapped to the same ZigBee attribute, the mapping will be incomplete.   | Update the UAM file to avoid overlap.           |
| UIC-1088  | Potentially wrong controller NIF after controller migration.                                          | Usually no problem in practice                  |

# Z-Wave Certification information
> _NOTE the ZPC is at this point not certifiable_

See [ZPC User Guide](readme_user.md)

