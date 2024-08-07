# Portable Runtime Release Notes

## [1.6.0] - Aug 2024

* Portable runtime cli and gui now supports Example Emulated Device(EED) to test without actual hardware.
* Reference platform updated to Bookworm 64-bit based Raspberry Pi OS.
  
## [1.5.0] - Feb 2024

* Portable runtime gui enhanced to work with latest unify core.
* Portable runtime cli and gui now supports ZIGPC-NCP, AOXPC, Multiprotocol (ZIGPC-RCP).
  Further details about the cli and gui in-detail changes can be found in documentation.
* Added failure log when firmware image is not present at the given path during flash app command.
* Chip type option check validation is fixed.
* Portable runtime supports a new resource-path option. (defaults to local packaged resources placed in      
  resources_local folder)

## [1.4.0] - Aug 2023

* New flash applications (SensorPIR, Wall Controller, Power Strip) support added
    Note : The binaries for these new applications must be added to the
             `resources/  app-binaries` directory manually, until they become publicly available in the next GSDK release
* `list-devices` command now enhanced to show configured RF region, board number and application name
* Refactored DSK reading and MFG token reading
* Made chip type optional for MFG token reading
* Moved `list-devices` old functionality to `list-devices-raw`
* Refactored function `get_serialnum`
* Docker containers are now all specifically named `unify-%service-name%`.

## [1.3.2] - Jun 2023

## [1.3.1] - Mar 2023

## [1.3.0] - Feb 2023

### Changes

* Added `flash-app` and `read-dsk` subcommands, for flashing application FW files to a board and to read out the DSK.
* Include a platform specific version of the Silicon Labs tools `Commander`, for providing the above feature.
* Changed the shorthand argument for chip-type, from `-d` to `-c`, as there was a clash
  between that option and the `--device` option.

## [1.2.1] - Sep 2022

## [1.2.0] - Aug 2022
