# Unify Release Notes

Below are descriptions of the changes to our shared components.
These changes affects all applications based on the Unify Host SDK.

For the full documentation of this latest release, go to
[Unify Host SDK documentation](https://siliconlabs.github.io/UnifySDK).

For the application specific release notes, please follow these links:

* [GMS](../applications/gms/release_notes.md)
* [NAL](../applications/nal/release_notes.md)
* [Dev GUI](../applications/dev_ui/dev_gui/release_notes.md)
* [Image Provider](../applications/image_provider/release_notes.md)
* [UPVL](../applications/upvl/release_notes.md)
* [ZigPC](../applications/zigpc/release_notes.md)
* [ZPC](../applications/zpc/release_notes.md)
* [UPTICap](../applications/upti_cap/release_notes.md)
* [UPTIWriter](../applications/upti_writer/release_notes.md)
* [AoXPC](../applications/aox/applications/aoxpc/release_notes.md)
* [AoX Positioning application](../applications/aox/applications/positioning/release_notes.md)
* [CPCd](../applications/cpcd/release_notes.md)
* [OpenThread Border Router](../applications/openthread_border_router/release_notes.md)
* [Zigbeed](../applications/zigbeed/release_notes.md)
* [Portable Runtime](../portable_runtime/release_notes.md)

## [1.6.0] - August 2024

### Added (1.6.0)

* Portable runtime cli and gui now supports Example Emulated Device(EED) to test without actual hardware.

### Changed (1.6.0)

* Reference platform updated to Bookworm 64-bit based Raspberry Pi OS.
* ZigPC migrated to Simplicity SDK v2024.6.0 with alpha quality.
* Mosquitto version updated to 2.0.18.

### Known Issues (1.6.0)

* **UIC-2261**: Attribute Mapper parent operator navigation does not always work (only on left hand sides).
  _Note_: Avoid using the parent operator in UAM files, use the common_parent_type scope setting instead.
* **UIC-2228**: Attribute Mapper reducer is disabled due to errors. The mapper will not successfully reduce constant expressions.
  _Note_: Avoid unnecessary calculations in the UAM files. e.g. write 100 instead of 10*10.
* **UIC-2725**: The Dotdot MQTT library does not always use enum names, even though the Unify Specification indicates that it should.
  _Note_: Be tolerant to numbers instead of strings for enums.

## [1.5.0] - Feb 2024

### Added (1.5.0)

* Portable runtime GUI is now being released as an artifact for windows and Linux.
* Added Description cluster with device type attribute to unify. (Experimental)
* Upgraded DoorLock cluster to match matter data model. (Experimental)
* Mosquitto version updated to 2.0.15.
* Docker
  * Rust version updated to 1.71.
  * Updated and documented the dependencies used in Unify Docker.
* CMake
  * Will throw build failure error if sl_status_string.sh generation failed.
  * GIT_VERSION_SHA variable will be populated during cmake build.(this variable is a part of final version string)

### Fixed (1.5.0)

* NetworkList State attribute section is now added to Unify Specification.
* Added handling for ExecuteIfOff parameter of move and movetolevel command of Level cluster.
* Removed ignorable error messages regarding file descriptors on terminal resize.
* Fixed asSnakeCaseUpper JSON helper function when several letters are uppercase in a row.
  Example: String AoXLocatorCTEMode -> was previosly AOX_LOCATORCTE_MODE
                                    -> After fix will be AOX_LOCATOR_CTE_MODE

### Known Issues (1.5.0)

* Supports GSDK version 4.3.0.
* The 1.5.0 protocol controllers MUST be used with the 1.5.0 libunify only (as a result of updating the JSON helper function).
* ZigPC 1.5.0 release is of Alpha quality.
* **UIC-2261**: Attribute Mapper parent operator navigation does not always work (only on left hand sides).
  _Note_: Avoid using the parent operator in UAM files, use the common_parent_type scope setting instead.
* **UIC-2228**: Attribute Mapper reducer is disabled due to errors. The mapper will not successfully reduce constant expressions.
  _Note_: Avoid unnecessary calculations in the UAM files. e.g. write 100 instead of 10*10.
* **UIC-2725**: The Dotdot MQTT library does not always use enum names, even though the Unify Specification indicates that it should.
  _Note_: Be tolerant to numbers instead of strings for enums.

## [1.4.0] - Aug 2023

### Added (1.4.0)

* Attribute Mapper can now create instances and clear values.

* Unify DotDot Attribute Store component can now handle the node state topic.
  Configuration struct was updated with a few more settings to accomodate for it.
* Added State topic and Group Cluster handling in the Unify DotDot Attribute Store compopnent
* Node state Cluster
  * Now supports Discover Security Command to read Security state of the Node
  * Network Status Attribute now supports new state "Commissioning started"
  * Secutiry Status Attribute now supports new "Matter" mode.
  * Network List Attribute is now added to the cluster

* GSDK version updated to 4.3.0

* Docker Changes
  * Remove unnecessary libyaml-cpp0.6. libyaml-cpp-dev is already pulling the related one.
      This change will help to support current debian-12 (which currently ships libyaml-cpp0.7)
  * Now docker pulls all git lfs assets before starting the build.

### Fixed (1.4.0)

* Attribute Mapper fixed to evaluate all type assignments for a given destination.
  The mapper used to "forget" some assignments if both the desired and reported
  values would be assigned by the same attribute update.
  Note that new mappings may become "active" all of a sudden with existing maps.
* Attribute resolver on giving up on reading reported attribute, will now remove the
  desired attribute value to have uniformity.

### Known Issues (1.4.0)

* **UIC-2261**: Attribute Mapper parent operator navigation does not always work (only on left hand sides)
  _Note_: Avoid using the parent operator in UAM files, use the common_parent_type scope setting instead
* **UIC-2228**: Attribute Mapper reducer is disabled due to errors. The mapper will not successfully reduce constant expressions.
  _Note_: Avoid unnecessary calculations in the UAM files. e.g. write 100 instead of 10*10
* **UIC-1725**: Resize a terminal with a running Unify application and a few error messages regarding file descriptors will be displayed.
  _Note_: The errors can be ignored, there is no consequence
* **UIC-1593**: The Dotdot MQTT library does not always use enum names, even though the Unify Specification indicates that it should.
  _Note_: Be tolerant to numbers instead of strings for enums.

## [1.3.2] - Jun 2023

## [1.3.1] - March 2023

### Added (1.3.1)

* Attribute Mapper updated to handle scope priorities
  * :exclamation: **NOTE**: make sure that existing maps have undefined banches
    in if statements, as they may trigger evaluations even if dependencies
    do not exist. For example:

    ```uam
    r'2 = if(r'3 > 0) 1 0
    ```

    If Attribute Type 3 does not exist, `if(r'3>0)` returns `false` and
    therefore the map above would assign the value 0 in `r'2`. Use something
    like this instead:

    ```uam
    r'2 = if(e'3 == 0) undefined
          if(r'3 > 0) 1 0
    ```

## [1.3.0] - February 2023

### Added (1.3.0)

* Attribute Mapper new functionalities:
  * Support for a set of built-in functions that can be used in UAM files.
  * Support for invididual scope configuration
  * Support for >= and <= operators
* MQTT client automatically configure the ApplicationMonitoring Will message when
  connecting to the broker.
* Shared Unify ApplicationMonitoring component allowing applications to
  configure their attributes.
* Experimental support for using Nix to handle dependencies, eventually allowing us to cut the ties with the build host.

### Changed (1.3.0)

* Reference platform updated to Bullseye 64-bit based Raspberry Pi OS.
  * _Note_: Migration from a 32-bit installation is not supported.
* Migrated to Gecko SDK version to v4.2.0.
* Updated Rust compiler from 1.60 to 1.64.
* MQTT Client ID configuration is now mandatory for applications, it is used
  for ApplicationMonitoring and defaults to the application name.
* The Debian packages libuic*.deb has been renamed to libunify*.deb, to reflect
  the name of the binary /usr/lib/libunify.so. Be aware that it should replace the
  old package during upgrade, but it might require a manual reinstall of the packages
  that depend on it - eg. uic-zpc or uic-zigpc.
* The shared library *libunify.so* is now also available as a static library *libunify.a*
  in the development package libunify-dev_*.deb.
* The Portable Runtime is packaged with the previous release of Unify SDK 1.2.1.
  * _Note_: The package can be updated by downloading the release artifact `unify_1.3.0_x86_64.zip` and replacing the old package in `./resources/docker-files/`.

## [1.2.1] - Sep 2022

### Changed (1.2.1)

* Attribute Store does not save sequentially to the datastore and now uses a
  contiki process. A new configuration allows to configure:
  * A cooldown timer, which will wait for this time after the last Attribute
    Store update before saving to the datastore.
  * A safety timer, which will save to the datastore regularly, in case the
    Attribute Store is being updated more often than the cooldown timer for a
    too long time.
* ZAP Generated files are now committed as source and ZAP is not run as part of
  the normal build step anymore. In order to generate ZAP files during build one
  must add the cmake option `-DZAP_GENERATE=ON` during cmake configuration.

### Fixes (1.2.1)

* Attribute Store Poll Engine did not remove attributes from the poll queue,
  when they were deleted causing a warning to be printed when a deleted
  attribute was to be polled.

## [1.2.0] - Aug 2022

### Added (1.2.0)

* Shared attribute store helpers to read/write ZCL attributes
* Portable Runtime * allows you to demo a Unify based Z-Wave IoT Gateway on your Windows, Mac or Linux Desktop with the help of Docker containers.
* Binaries for x86_64 has been released * they are unsupported for now.
* New Debian packages:
  * libuic-dev - Unify development header files.
  * uic-cpcd - Silabs Co-Processor Communication Daemon for Linux.
  * uic-cpc-hci-bridge - Bluetooth on host, via chip managed by CPCd.
  * uic-otbr - OpenThread Border Router, via chip managed by CPCd.
  * uic-zigbeed - Zigbee stack on host, via chip managed by CPCd (armhf only).

### Changed (1.2.0)

* Attribute Store can be configured runtime to:
  * Save automatically all changes to the Datastore
  * Perform validation of add/write operations based on attribute type registrations
* Attribute Mapper/Utils understand automatically all numerical types in the
attribute store based on type registration.
* Code depending on [GeckoSDK](https://github.com/SiliconLabs/gecko_sdk) has been bumped to the latest release version 4.1.0.
* The Debian packages have been put in a combined Zip package - eg. `unify_x.y.z_armhf.zip`.
* Rust version bumped from 1.58.1 to 1.60.0.
* Documentation is now generated by [Sphinx](https://www.sphinx-doc.org/) with a [Read The Docs](https://readthedocs.org/) theme, similar to subpages on [](docs.silabs.com).
* Build changes:
  * Default builds now includes building CPCd, OTBR and Zigbeed. Relevant build flags to disable these are BUILD_CPCD, BUILD_ZIGBEED and BUILD_OTBR.
  * When building CPCd the [cpc-daemon GitHub repo](https://github.com/SiliconLabs/cpc-daemon) is automatically fetched by CMake.
  * When building AoX, OTBR, Zigbeed, ZigPC the [GeckoSDK GitHub repo](https://github.com/SiliconLabs/gecko_sdk) is automatically fetched by CMake.

### Fixed (1.2.0)

* The naming convention of MQTT client IDs is updated for all Unify applications to `<unify_application>_<pid-of-application>`. Previously this was `uic_<incremented-number>`.

## [1.1.1] - Mar 2022

### Fixed (1.1.1)

* Running with command line parameter`--dump-config` now dumps proper YAML.
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

### Added (1.1.0)

* A new component, Gecko Bootloader Interface (uic_gbl_interface), for performing OTA updates
* A new component, Attribute Polling Engine (unify_attribute_poll), supplies the functionality of polling attribute reported values with a supplied interval.
* A new application, Name and Location (nal), for devices name and location management.
* A new application, Unify Packet Trace Interface Capture (upti_cap), to communicate with Silicon Labs WSTK adapters and publish debug channel messages to a MQTT broker.
