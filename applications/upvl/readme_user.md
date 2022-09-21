# UPVL User's Guide

UPVL is an acronym for _Unify Framework Provisioning List_

## Description

The UPVL MQTT Client is an
application that manages the SmartStart List topic `ucl/SmartStart/List`.

The UPVL is required for all/any of the protocol controllers to support
SmartStart.

It listens to the following MQTT topics, which is used to modify the SmartStart
List and publish the list to `ucl/SmartStart/List` whenever it was updated:

* `ucl/SmartStart/List/Update`
* `ucl/SmartStart/List/Remove`

The SmartStart List is persisted in the file system by the UPVL and is published
whenever the UPVL connects to the MQTT broker (i.e., when the UPVL or MQTT
broker restarts, the persisted SmartStart list is published to
`ucl/SmartStart/List`).

## Installation

For installing please refer to the general installation method in
[Getting started](../../doc/getting_started.md).

## Running the UPVL

### Using Systemd Service

The best way to run the UPVL is using the Systemd service that is installed with
the Debian installer. For more information, see
[Unify Framework User guide](../../doc/unify_readme_user.md).

### Using Command Line

Alternatively, the UPVL may be run by executing `uic-upvl`. It is possible to
configure the MQTT broker Hostname or IP, UPVL database path, through command line
options. For more details about the options, run `uic-upvl --help`.

Make sure that you do not run the UPVL both as a service and using the
command line.

```console
pi@unify:~ $ service uic-upvl status
● uic-upvl.service - Unify Framework Provisioning List
     Loaded: loaded (/lib/systemd/system/uic-upvl.service; enabled; vendor preset: enabled)
     Active: active (running) since Tue 2022-02-01 13:32:57 CET; 20h ago
   Main PID: 1736 (uic-upvl)
      Tasks: 1 (limit: 4624)
     Memory: 3.2M
     CGroup: /system.slice/uic-upvl.service
             └─1736 /usr/bin/uic-upvl --upvl.db_file /var/lib/uic/upvl.db

Feb 02 09:20:06 ubunty200402 uic-upvl[1736]: 2022-Feb-02 09:20:06.343484 <i> [upvl_mqtt] Disconnected from MQTT broker. Status: 7.
Feb 02 09:20:06 ubunty200402 uic-upvl[1736]: Published 0 messages.
Feb 02 09:20:06 ubunty200402 uic-upvl[1736]: Received 0 messages of which 0 were dropped.
Feb 02 09:20:07 ubunty200402 uic-upvl[1736]: 2022-Feb-02 09:20:07.344734 <i> [upvl_mqtt] (Re-)connected to broker
Feb 02 09:20:07 ubunty200402 uic-upvl[1736]: 2022-Feb-02 09:20:07.344815 <i> [upvl_mqtt] Subscribing
Feb 02 09:20:08 ubunty200402 uic-upvl[1736]: 2022-Feb-02 09:20:08.345994 <i> [upvl_mqtt] Disconnected from MQTT broker. Status: 7.
Feb 02 09:20:08 ubunty200402 uic-upvl[1736]: Published 0 messages.
Feb 02 09:20:08 ubunty200402 uic-upvl[1736]: Received 0 messages of which 0 were dropped.
Feb 02 09:20:09 ubunty200402 uic-upvl[1736]: 2022-Feb-02 09:20:09.347998 <i> [upvl_mqtt] (Re-)connected to broker
Feb 02 09:20:09 ubunty200402 uic-upvl[1736]: 2022-Feb-02 09:20:09.348156 <i> [upvl_mqtt] Subscribing

```

If the service is running, stop it using the following command before
you run the application manually:

```console
pi@unify:~ $ service uic-upvl stop
```

## Troubleshooting

* _Inclusion of a device keeps failing_ try to reset the UPVL database:

  ``` sh
  sudo systemctl stop uic-upvl && rm /var/lib/uic/upvl.db && sudo systemctl start uic-upvl
  ```
