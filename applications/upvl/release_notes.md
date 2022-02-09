# UPVL Release Notes

## [1.1.0] - Feb 2022

### Added

### Changed

* UPVL datastore file key has been changed from --upvl.db_file to --upvl.datastore_file
* Default datastore file has been changed from /var/lib/uic/upvl_test.db to /var/lib/uic/upvl.db
** If you used the default configuration, keep your previous database by running `mv /var/lib/uic/upvl_test.db /var/lib/uic/upvl.db`
* Switched to use the Unify MQTT component so that on restart the MQTT connection can be properly resumed.

### Fixed

## [1.0.3] - Nov 2021

## [1.0.2] - Oct 2021

## Known Bugs
