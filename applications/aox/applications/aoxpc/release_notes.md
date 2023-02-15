# AoXPC Release Notes

## [1.3.0] - Feb 2023

### Added (1.3.0)

* Enabled the minimum required attributes for ApplicationMonitoring.

## [1.2.1] - Sep 2022

The AoXPC relies on the latest Gecko SDK release. For a complete list of release notes,
see [Bluetooth SDK](https://www.silabs.com/documents/public/release-notes/bt-software-release-notes-4.1.0.0.pdf)
and [Bluetooth Location Services](https://www.silabs.com/documents/public/release-notes/bt-rtl-library-release-notes-4.1.0.0.pdf).

## [1.2.0] - Aug 2022

The AoXPC relies on the latest Gecko SDK release. For a complete list of release notes,
see [Bluetooth SDK](https://www.silabs.com/documents/public/release-notes/bt-software-release-notes-4.0.0.0.pdf)
and [Bluetooth Location Services](https://www.silabs.com/documents/public/release-notes/bt-rtl-library-release-notes-4.0.0.0.pdf).

Some of the highlights are listed below.

### Changed (1.2.0)

* Take most shared resources from Gecko SDK

### Added (1.2.0)

* NCP firmware update for the antenna array board
* Support for new dual-polarized antenna array board BRD4191A
* Angle standard deviation values


## [1.1.1] - Mar 2022

### Added (1.1.1)

* Publish the EndpointIdList attribute under the State for each node.

### Fixed (1.1.1)

* Fix attributes not published to MQTT on first startup issue.
* Fixed incorrect RTL lib antenna switch pattern for reduced antenna arrays (3x3 URA, 1x4 ULA).

## [1.1.0] - Feb 2022

### Added (1.1.0)

* Support for AoX Locator configuration via config file or via MQTT.
* Support for parsing AngleCorrection commands
* Support for publishing AngleReport commands
* Persistent storage of the AoX Locator configuration
