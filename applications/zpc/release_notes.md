# ZPC Release Notes

## [1.6.0] - Aug 2024

**BREAKING** : ZPC database version bumped from 2 to 3.

This change allows us to align the different implementations of command classes. It should be easier to maintain and to address issues.

A new tool have been deployed in the zpc debian package that is automatically called when you are installing it. It will look for the default database location (default db : `/var/lib/uic/zpc.db`) of your config file (default config path : `/etc/uic/uic.cfg`)

You can also call the tool manually after installing the zpc package :

`/usr/bin/zpc_database_upgrade_tool --target_version 3 --zpc.datastore_file $YOUR_ZPC_DB_PATH`

### Added (1.6.0)

* New command classes support :
  * Thermostat Operating State
  * Thermostat Fan State
  * Thermostat Fan Control Mode
  * Humidity Control Setpoint
  * Humidity Control Operating State
  * Humidity Control Mode

### Changes (1.6.0)

* Thermostat Setpoint update
  * Should be more consistent and easier to use.
  * Supports bitmask B implementation
  * Can support all Z-Wave precision & scale (°C & °F)
  * Always expose the temperature in °C with precision of 2 (as per cluster definition)
* New documentation of how to interact with most of the command classes.
* Updated dev documentation of Cluster implementation for Z-Wave
* Updated dev documentation of Implementing of Command Classes
* Reference platform updated to Bookworm 64-bit based Raspberry Pi OS
* Migrated to Simplicity SDK version to v2024.6.0

### Fixed (1.6.0)

* [Fix ZPC Polling mechanism](https://github.com/SiliconLabs/UnifySDK/pull/45)
* [Send 0xFF by default for endpoint_find](https://github.com/SiliconLabs/UnifySDK/pull/37)
  * Revert regression introduced in 1.4.0
  * A flag allows you to send specific value instead
* [MAX_PING_TIME_INTERVAL should be 24 hours, 24 not minutes](https://github.com/SiliconLabs/UnifySDK/pull/40)
* [Fix infinite loop on invalid next configuration parameter.](https://github.com/SiliconLabs/UnifySDK/pull/27)
* [Prioritize command classes for interview](https://github.com/SiliconLabs/UnifySDK/pull/39)

### Known issues (1.6.0)

* **UIC-3335** : In some cases Z-Wave TX queue can lock itself.
  * No workaround for now. If you encounter this issue yourself, please fill a Saleforce ticket or a github issue.
  * A fix should be published on github before next release.

## [1.5.0] - Feb 2024

### Added (1.5.0)

* Added SoundSwitch Command Class
* Added preliminary support for gcc-12
* Added Base structure of python scripts to interact with CTT
* Support 3 bit bitmask for notification types report

### Changes (1.5.0)

* Document the process for implementing a new UCL cluster and zwave command classes

### Fixed (1.5.0)

* Fix COMMAND_CLASS_VERSION value for Rust components
* Fix support for COMMAND_CLASS_METER V1,V2 & V3
* Fix Multi Channel Handling of Secure NIF
* Fix Multi-Channel End Point Find command
* Fix CC SWITCH_COLOR Duration param

## [1.4.0] - Aug 2023

### Added (1.4.0)

* Enabled ForceReadAttributes commands.
* Added control of the User Code Command Class, version 2.
* Added a state command allowing to re-discover S0/S2 Security keys.
* Added an IlluminanceMeasurement mapping from Multilevel Sensor CC
* Implement Control of the Door Lock Command Class, v4

### Changed (1.4.0)

* Enabled strict security filtering for support of Command Classes always in the NIF.
  Command Classes always present in the NIF used to be accepted no matter what.
  Command Classes always present in the NIF are now only accepted at the highest shared security level with the sending node.
* Split unify dotdot attribute store attribute publishing config into desired and reported attribute publishing config
* Add the mapper the capability to clear attributes
* Force UIC services to run in /var/lib/uic

### Fixed (1.4.0)

* Z-Wave API Transport recovers from aborted transactions when an unexpected
  Z-Wave API started is received.
* Fix some potential s0 buffer overflows

### Known Issues (1.4.0)

* **UIC-2274**
  * During installation, The presented dialog to specify the serial-port for the
    given application does not update the actual configuration file
    (/etc/uic/uic.cfg). The default value is written instead.
  * Packages may overwrite configuration parameters during installation of none
    related packages. Updated documentation to mitigate this issue.

* **UIC-2219**:
  Bindings command processing do not validate the full UNID.
  This may trigger to establish spurious associations if having several
  Z-Wave networks.
  _Note_: Do not try to bind nodes accross networks. (inter-pc bindings)

* **UIC-2283**:
  Color Switch Command Class durations are completely ignored.
  _Note_: Set the colors instantaneously to avoid issues.

* **UIC-1779**:
  Multi Channel Endpoint Find Reports reports to follow set
  to > 0 are not handled and will trigger a SPAN increase out-of-sync situation.
  _Note_: No workaround, wait for the nodes to recover.

* **UIC-1652**:
  The ZPC silently ignores incorrect YAML and uses default values
  if it cannot parse the YAML content for its configuration.
  _Note_: Verify the validity of the YAML in the configuration file.

* **UIC-1088**:
  Potentially wrong controller NIF after controller migration.
  _Note_: Usually not a problem in practice

## [1.3.1] - Mar 2023

### Added (1.3.1)

* Added a few Multilevel Sensor Command Class maps.

### Changed (1.3.1)

* Refactored the UAM files to work with the Attribute Mapper scope priority
* Refactored the Multilevel Switch Command Class to be simpler.

### Fixed (1.3.1)

* Removed a race condition that occurs if being reset while including a
  node using SmartStart

## [1.3.0] - Feb 2023

### Added (1.3.0)

* Enabled the minimum required attributes for ApplicationMonitoring.
* Added "full" support for the Scene Cluster, including the Scene Table for devices
  supporting SceneRequired ZCL attributes.
* Added Control of Barrier Operator Command Class
* Added Control of Scene Activation Command Class
* Added Control of Scene Actuator Configuration Command Class
* Added Control of Scene Controller Configuration Command Class
* Added mappings to more attributes for the Level Cluster
* Added a few helper cli commands:
  * Reset MPAN/SPAN
  * Export Security keys to Zniffer format
* Added a Z-Wave LR Max Power Configuration
* Added a few Device Specific UAM maps:
  * ZDB5100
  * Aeotec MultiSensor 7

### Changed (1.3.0)

* Updated the Color Switch mapping from HSL to HSV. Resulting color will differ
  for a given set of Level/Hue/Saturation values.
* Refactored UAM default files using the new Attribute Mapper functionalities
* Updated cluster Attribute behaviors:
  * Level cluster store and use previous level, if OnOff cluster (simulation) is used to turn Level device on/off
  * OnOff cluster changes to Off, when Level cluster Current Level changes to 0, when OnOff cluster is simulated

### Fixed (1.3.0)

* Fixed an issue with S0 transport, that would lock up if it needed to send
  streamed messages
* SetParameter Commands sent via MQTT will also attempt to discover parameters
  for Configuration CC v1-v2 supporting nodes.
* Updated the Tx Queue to be able to break back-offs when transports
  need to send a frame despite expecting incoming frames
* IdentifyTime/Level RemainingTimes are now countdowns instead of fixed values

## [1.2.1] - Sep 2022

### Changed (1.2.1)

* Allowed UAM files to define the desired value of the Wake Up Interval.
* Attribute Store is now configured to back-up to the datastore 10 seconds
  after the last Attribute Store update, and every 10 minutes if the attribute
  Store is being changed more often than every 10 seconds for 10 minutes in a
  row.
  Note that the Attribute Store is always saved at shutdown.

### Fixed (1.2.1)

* Network management waits for NL nodes to be awake before initiating return
  routes / neighbor discovery operations
* Tx Queue applies back-off when network management sends frames to destinations
* Tx Queue applies back-off when we received unsolicited routed frames.
* (Multi Channel) Association Remove/Set commands will be attempted only 2 times
  if the end node does not accept values.
* Network Management will not accept new operations once the Z-Wave Controller
  has initiated a reset.
* Attribute Store Poll Engine did not remove attributes from the poll queue,
  when they were deleted causing a warning to be printed when a deleted
  attribute was to be polled.

## [1.2.0] - Aug 2022

_Note_: GA releases are not certified. The ZPC will be Z-Wave certified in the
1.2.1 release.

### Added (1.2.0)

* ZPC can recover and operate networks if started without any datastore
  * DSK and Kex Fail type will be missing
  * Granted keys and inclusion protocols will be discovered again

* New Command Classes:
  * Partial control of the Color Switch Command Class, version 3
  * Partial control of the Meter Command Class, version 6
  * Support for the Binding cluster, which in turn will add/remove Z-Wave Associations
  * Control of the Alarm Sensor Command Class.

* PTI enabling/disabling via the RF Telemetry cluster.

### Changed (1.2.0)

* Associations and Wake Up are configured as early as possible during the
  interview, to minimize damage if sleeping nodes fall asleep during the interview.

* ZCL attributes are now stored with their real storage type instead of int.

### Fixed (1.2.0)

* Tx Queue gets flooded with NOPs if a few nodes are failing in the network.
* Tx Queue backs off too much in some cases.

## [1.1.1] - Mar 2022

### Added (1.1.1)

* Full control for Configuration Command Class, version 4
* Added generated OnOff cluster commands for nodes issuing Basic Set commands to the ZPC.

### Changed (1.1.1)

* Changed configuration keys for OTA config, if these have been specified in
  the ZPC config file, they need to be updated to the new keys manually
  * Changed `zpc.ota_chache_path` to `zpc.ota.cache_path`
  * Changed `zpc.ota_chache_size` to `zpc.ota.cache_size`

* Default get retry timer is lowered to 3s instead of 20s to mitigate against Wake
  Up nodes falling asleep during node interviews.

### Fixed (1.1.1)

* ZPC gets stuck at reset in some rare cases.
* ZPC stops controlling OnOff if Binary Switch Set without supervision failed.
* ZPC stops granting S2 keys in some cases after a reset.
* ZPC does not let Iot Services control/remove a node after SmartStart self-destruct failure.
* ZPC consider Wake Up node failings if they fell asleep while communicating with them
* Re-interview a Wake Up leads to a difficult to recover situation
* Not all MQTT publications unretained after a node exclusion.
* S2 DSK of the previous node saved again with the next node if it does not support S2.

## [1.1.0] - Feb 2022

### Added (1.1.0)

* Partial Control for Central Scene Command Class
  * Populating Scene Attributes to reflect the node's capabilities (total number of scenes)
  * Central Scene is mapped to the Scene Cluster.

* Issuing Generated Commands based on interaction with Z-Wave nodes
  * Issuing RecallScene when receiving Central Scene Notifications.
  * Issuing RF Telemetry reports at each transmission when RF Telemetry is enabled

* Support for RF Telemetry (reporting transmissions events)
* Monitoring if Offline nodes are responding again with regular NOPs commands.
* Network Polling

### Changed (1.1.0)

* ZPC datastore file key has been changed from --datastore.file to --zpc.datastore_file
* Default datastore file has been changed from /var/lib/uic/database.db to /var/lib/uic/zpc.db
  * If you used the default configuration, keep your previous database by running `mv /var/lib/uic/database.db /var/lib/uic/zpc.db`

* Increased Z-Wave Tx Queue robustness to multiple Tx sessions.
* Communication with Z-Wave Module optimized

### Fixed (1.1.0)

* Interviewing an offline node would make it appear online.
* Missing Wake Up Interval Set during interview for door/window sensor
* ZPC hangs and stops resolving attributes when DevGUI sends a MoveWithOnOff command

## [1.0.3] - Nov 2021

## [1.0.2] - Oct 2021

### Added (1.0.2)

* ZPC is Z-Wave Certified against the Z-Wave Specification release 2021A.
* Support of Z-Wave Command Classes
  * Firmware Update
* Control of Z-Wave Command Classes
  * Firmware Update
  * Indicator

### Changed (1.0.2)

* OTA UIID has been updated to include hardware revision

### Fixed (1.0.2)

* Fixed various certification issues

## [1.0.1] - Aug 2021

### Fixed (1.0.1)

* Build issues

## [1.0.0] - Aug 2021

### Added (1.0.0)

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

### Changed (1.0.0)

* Various bug fixes
* UAM Language updates
* Documentation updates

## Z-Wave Certification information

See [ZPC Certification Guide](readme_certification.md)

## Devices tested with the ZPC

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
| Aeotec           | MultiSsensor 7                | x                              | S2 Authenticated   |
| Hank             | Door/Window Sensor            | x                              | S2 Unauthenticated |
| Hank             | RGB Bulb                      | x                              | Non-Secure         |
| Hank             | Smart plug                    | x                              | S0                 |
| Fibaro           | Fibaro Flood Sensor           | x                              | S0                 |
| Fibaro           | Heat Controller               | x                              | S2 Authenticated   |
| Philio           | Smart energy plug-in switch   | x                              | S0                 |
| Secure           | Smart Plug 302                | x                              | Non-Secure         |
| Ring             | Alarm motion detector (1 Gen) | -                              | S2 Authenticated   |
| Yale real living | Door Lock                     | x                              | S0                 |
| Alarm.com        | Smart water valve             | x                              | S2 Authenticated   |
| Alarm.com        | Dry contact sensor            | -                              | Non-Secure         |
