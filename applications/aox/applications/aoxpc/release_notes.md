# AoXPC Release Notes

## [1.1.1] - Mar 2022

### Added

* Publish the EndpointIdList attribute under the State for each node.

### Fixed

* Fix attributes not published to MQTT on first startup issue.
* Fixed incorrect RTL lib antenna switch pattern for reduced antenna arrays (3x3 URA, 1x4 ULA).

## [1.1.0] - Feb 2022

### Added

* Support for AoX Locator configuration via config file or via MQTT.
* Support for parsing AngleCorrection commands
* Support for publishing AngleReport commands
* Persistent storage of the AoX Locator configuration
