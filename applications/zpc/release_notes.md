# ZPC Release Notes

## [1.2.0] - Aug 2022

_Note_: GA releases are not certified. The ZPC will be Z-Wave certified in the
1.2.1 release.

**Added**:

* ZPC can recover and operate networks if started without any datastore
  * DSK and Kex Fail type will be missing
  * Granted keys and inclusion protocols will be discovered again

* New Command Classes:
  * Partial control of the Color Switch Command Class, version 3
  * Partial control of the Meter Command Class, version 6
  * Support for the Binding cluster, which in turn will add/remove Z-Wave Associations
  * Control of the Alarm Sensor Command Class.

* PTI enabling/disabling via the RF Telemetry cluster.

**Changed**:

* Associations and Wake Up are configured as early as possible during the
  interview, to minimize damage if sleeping nodes fall asleep during the interview.

* ZCL attributes are now stored with their real storage type instead of int.

**Fixed**:

* Tx Queue gets flooded with NOPs if a few nodes are failing in the network.
* Tx Queue backs off too much in some cases.

## [1.1.1] - Mar 2022

**Added**:

* Full control for Configuration Command Class, version 4
* Added generated OnOff cluster commands for nodes issuing Basic Set commands to the ZPC.

**Changed**:

* Changed configuration keys for OTA config, if these have been specified in
  the ZPC config file, they need to be updated to the new keys manually
  * Changed `zpc.ota_chache_path` to `zpc.ota.cache_path`
  * Changed `zpc.ota_chache_size` to `zpc.ota.cache_size`

* Default get retry timer is lowered to 3s instead of 20s to mitigate against Wake
  Up nodes falling asleep during node interviews.

**Fixed**:

* ZPC gets stuck at reset in some rare cases.
* ZPC stops controlling OnOff if Binary Switch Set without supervision failed.
* ZPC stops granting S2 keys in some cases after a reset.
* ZPC does not let Iot Services control/remove a node after SmartStart self-destruct failure.
* ZPC consider Wake Up node failings if they fell asleep while communicating with them
* Re-interview a Wake Up leads to a difficult to recover situation
* Not all MQTT publications unretained after a node exclusion.
* S2 DSK of the previous node saved again with the next node if it does not support S2.

## [1.1.0] - Feb 2022

**Added**:

* Partial Control for Central Scene Command Class
  * Populating Scene Attributes to reflect the node's capabilities (total number of scenes)
  * Central Scene is mapped to the Scene Cluster.

* Issuing Generated Commands based on interaction with Z-Wave nodes
  * Issuing RecallScene when receiving Central Scene Notifications.
  * Issuing RF Telemetry reports at each transmission when RF Telemetry is enabled

* Support for RF Telemetry (reporting transmissions events)
* Monitoring if Offline nodes are responding again with regular NOPs commands.
* Network Polling

**Changed**:

* ZPC datastore file key has been changed from --datastore.file to --zpc.datastore_file
* Default datastore file has been changed from /var/lib/uic/database.db to /var/lib/uic/zpc.db
  * If you used the default configuration, keep your previous database by running `mv /var/lib/uic/database.db /var/lib/uic/zpc.db`

* Increased Z-Wave Tx Queue robustness to multiple Tx sessions.
* Communication with Z-Wave Module optimized

**Fixed**:

* Interviewing an offline node would make it appear online.
* Missing Wake Up Interval Set during interview for door/window sensor
* ZPC hangs and stops resolving attributes when DevGUI sends a MoveWithOnOff command

## [1.0.3] - Nov 2021

## [1.0.2] - Oct 2021

**Added**:

* ZPC is Z-Wave Certified against the Z-Wave Specification release 2021A.
* Support of Z-Wave Command Classes
  * Firmware Update
* Control of Z-Wave Command Classes
  * Firmware Update
  * Indicator

**Changed**:

* OTA UIID has been updated to include hardware revision

**Fixed**:

* Fixed various certification issues

## [1.0.1] - Aug 2021

**Fixed**:

* Build issues

## [1.0.0] - Aug 2021

**Added**:

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

**Changed**:

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

* **UIC-2232**:
  A node with capabilities mapping to Level and OnOff cluster
  may stop responding to Toggle commands after a MoveToLevelWithOnOff command.
  *Note*: Avoid using Toggle and use On or Off instead.

* **UIC-2222**/**UIC-2230**:
  When routing is involved to reach a destination, the interview
  get-retries are too aggressive and create multi-hop collisions.
  *Note*: Increase the get_retry_timeout in the zpc_attribute_resolver if this
  is a problem. Note that sleeping nodes have a higher chance to fall asleep
  during an interview with high values.

* **UIC-2219**:
  Bindings command processing do not validate the full UNID.
  This may trigger to establish spurious associations if having several
  Z-Wave networks.
  *Note*: Do not try to bind nodes accross networks. (inter-pc bindings)

* **UIC-2218**:
  Start and stop the ZPC quickly may lead to a few leftover MQTT topics not removed.
  *Note*: Wait a few seconds before stopping the ZPC.

* **UIC-2213**:
  When a sleeping device supports Battery Command Class version 3,
  the ZPC does not finish interviewing the device and continuously send Battery
  Health Get commands when the device wakes up.
  *Note*: Wait for all the retries to complete

* **UIC-2211**:
  If a node does not respond to Supervision Get with an
  Association Set Command and does not establish the association, the ZPC will
  go into an infinite loop of trying to set the association
  *Note*: Remove the lifeline establishment of such a device with a UAM file,
  if one exists.

* **UIC-1927**:
  Color Switch Command Class durations are completely ignored.
  *Note*: Set the colors instantaneously to avoid issues.

* **UIC-1780**:
  Security 0 Supported Commands Report reports to follow set
  to > 0 are not handled and will trigger a SPAN increase out-of-sync situation.
  *Note*: No workaround, wait for the nodes to recover.

* **UIC-1779**:
  Multi Channel Endpoint Find Reports reports to follow set
  to > 0 are not handled and will trigger a SPAN increase out-of-sync situation.
  *Note*: No workaround, wait for the nodes to recover.

* **UIC-1778**:
  Multi Channel Association / Association Report reports to follow set
  to > 0 are not handled and will trigger a SPAN increase out-of-sync situation.
  *Note*: No workaround, wait for the nodes to recover.

* **UIC-1652**:
  The ZPC silently ignores incorrect YAML and uses default values
  if it cannot parse the YAML content for its configuration.
  *Note*: Verify the validity of the YAML in the configuration file.

* **UIC-1271**
  IdentifyTime from the Identify cluster stays fixed, even
  though it should be a countdown.
  *Note*: Avoid using large IdentifyTime values.

* **UIC-1162**:
  When a device have multiple Z-Wave functionalities mapped to
  the same Zigbee attribute, the state of the device will be ambiguous.
  The ZCL state will represent the functionality that was last updated.
  *Note*: Update tha UAM file with a device fingerprint to circumvent issues.

* **UIC-1088**:
  Potentially wrong controller NIF after controller migration.
  *Note*: Usually not a problem in practice

* **UIC-729**
  Assigned Group cluster attributes are erased at re-interview.
  *Note*: Re-setup all group membership after a node re-interview

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
