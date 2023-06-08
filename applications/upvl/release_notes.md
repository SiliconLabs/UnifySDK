# UPVL Release Notes

## [1.3.2] - Jun 2023

## [1.3.1] - Mar 2023

## [1.3.0] - Feb 2023

## [1.2.1] - Sep 2022

## [1.2.0] - Aug 2022

## [1.1.1] - Mar 2022

## [1.1.0] - Feb 2022

### Changed (1.1.0)

* UPVL datastore file key has been changed from --upvl.db_file to --upvl.datastore_file
* Default datastore file has been changed from /var/lib/uic/upvl_test.db to /var/lib/uic/upvl.db
** If you used the default configuration, keep your previous database by running `mv /var/lib/uic/upvl_test.db /var/lib/uic/upvl.db`
* Switched to use the Unify MQTT component so that on restart the MQTT connection can be properly resumed.

## [1.0.3] - Nov 2021

## [1.0.2] - Oct 2021
