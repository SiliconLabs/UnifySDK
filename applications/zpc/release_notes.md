# ZPC Release Notes
## [1.3.2] - Jun 2023
### Fixed (1.3.2)
* Z-Wave API Transport recovers from aborted transactions when an unexpected
  Z-Wave API started is received.

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

## Known Bugs

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

* **UIC-2218**:
  Start and stop the ZPC quickly may lead to a few leftover MQTT topics not removed.
  _Note_: Wait a few seconds before stopping the ZPC.

* **UIC-1927**:
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

* **UIC-1162**:
  When a device have multiple Z-Wave functionalities mapped to
  the same Zigbee attribute, the state of the device will be ambiguous.
  The ZCL state will represent the functionality that was last updated.
  _Note_: Update tha UAM file with a device fingerprint to circumvent issues.

* **UIC-1088**:
  Potentially wrong controller NIF after controller migration.
  _Note_: Usually not a problem in practice

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
