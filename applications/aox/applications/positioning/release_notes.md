# AoX Positioning application Release Notes

## [1.2.0] - Aug 2022

The AoX Positioning application relies on the latest Gecko SDK release. For a complete list of release notes,
see [Bluetooth SDK](https://www.silabs.com/documents/public/release-notes/bt-software-release-notes-4.0.0.0.pdf)
and [Bluetooth Location Services](https://www.silabs.com/documents/public/release-notes/bt-rtl-library-release-notes-4.0.0.0.pdf).

Some of the highlights are listed below.

**Changed**:

* Take most shared resources from Gecko SDK

## [1.1.1] - Mar 2022

**Added**:

* Configuration option to enable/disable angle correction feedback feature.
* Publish the EndpointIdList attribute under the State for each node for asset tags.

## [1.1.0] - Feb 2022

**Added**:

* Support for reading AoX locators configuration using MQTT.
* Support for parsing AngleReports commands from AoX locators
* Support for publishing AngleCorrection commands to AoX locators
* Support for publishing EstimatedPosition via MQTT.
