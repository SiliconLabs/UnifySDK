# AoXPC Release Notes

## [1.1.0] - Feb 2022

### Added

* Support for AoX Locator configuration via config file or via MQTT.
* Support for parsing AngleCorrection commands
* Support for publishing AngleReport commands
* Persistent storage of the AoX Locator configuration

# Known Issues

| Known Issue | Workaround |
|-------------|------------|
| If `--aoxpc.datastore_file` option points to a file that doesn't exist, and `--aoxpc.config` is not provided either, then the AoXLocator attributes are not published. | Restart AoXPC with the same options (during the first run, the datastore file is created). |
