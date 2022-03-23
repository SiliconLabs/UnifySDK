# Unify Release Notes

The following links provides release notes for individual packages of Unify SDK.

- [GMS](applications/gms/release_notes.md)
- [NAL](applications/nal/release_notes.md)
- [Dev GUI](applications/dev_ui/dev_gui/release_notes.md)
- [Image Provider](applications/image_provider/release_notes.md)
- [UPVL](applications/upvl/release_notes.md)
- [ZigPC](applications/zigpc/release_notes.md)
- [ZPC](applications/zpc/release_notes.md)
- [UPTICap](applications/upti_cap/release_notes.md)
- [UPTIWriter](applications/upti_writer/release_notes.md)
- [AoXPC](applications/aox/applications/aoxpc/release_notes.md)
- [AoX Positioning application](applications/aox/applications/positioning/release_notes.md)

## Unify SDK shared components release notes

This document describes changes which affects all applications based on the Unify SDK. Below are the changes between the released versions of the Unify SDK and the current release.

## [1.1.1] - Mar 2022

### Fixed

- Running with command line parameter`--dump-config` now dumps proper YAML.
Before it would only dump max 2 level objects, e.g. `zpc.poll.xxx`
parameters was before dumped as:

```yaml
  zpc:
    poll.attribute_list_file: "/usr/share/uic/zwave_poll_config.yaml"
    poll.backoff: 30
    poll.default_interval: 60
```

and now it is dumped as:

```yaml
  zpc:
    poll:
      attribute_list_file: "/usr/share/uic/zwave_poll_config.yaml"
      backoff: 30
      default_interval: 60
```

## [1.1.0] - Feb 2022

### Added

* A new component, Gecko Bootloader Interface (uic_gbl_interface), for performing OTA updates
* A new component, Attribute Polling Engine (unify_attribute_poll), supplies the functionality of polling attribute reported values with a supplied interval.
* A new application, Name and Location (nal), for devices name and location management.
* A new application, Unify Packet Trace Interface Capture (upti_cap), to communicate with Silicon Labs WSTK adapters and publish debug channel messages to a MQTT broker.

### Changed

### Fixed

### Known issues
