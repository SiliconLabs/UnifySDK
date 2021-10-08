# ZPC Release Notes
This document describes the changes between the last released ZPC and the current release.

## [1.0.2] - Oct 2021
### Added
* ZPC is Z-Wave Certified against the Z-Wave Specification release 2021A.
* Support of Z-Wave Command Classes
  + Firmware Update
* Control of Z-Wave Command Classes
  + Firmware Update
  + Indicator

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
| UIC-1162  | When device have multiple functionalities on the Z-Wave end,which is mapped to the same ZigBee attribute, the mapping will be incomplete.   | Update the UAM file to avoid overlap.           |
| UIC-1088  | Potentially wrong controller NIF after controller migration.                                          | Usually no problem in practice                  |
| UIC-1175  | ZPC performs S2 re-synchronisation of all nodes after reboot.                                         | After a reboot the ZPC will re-negotiate SPANs and MPANs for all nodes at the first transmission attempt.                 |

# Z-Wave Certification information
> _NOTE the ZPC is at this point not certifiable_

See [ZPC User Guide](readme_user.md)

