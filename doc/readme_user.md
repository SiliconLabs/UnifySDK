# Unify User guide

This guide contains information which is common to all Unify applications
Details about each application can be found in the application specific 
user guides

## Prerequisites

Before installing the Unify SDK components on a Raspberry Pi
4 (RPi4), the following prerequisites shall be fulfilled:

- An RPi4 preinstalled with [Raspberry Pi OS Linux Version 10, Buster](https://downloads.raspberrypi.org/raspbian/images/raspbian-2020-02-14) 
- One of the following, supported wireless radios is connected to the RPi4 via
  USB:
  - UZB 7 serial dongle for Z-Wave
  - EFR32MG12 running NCP Firmware for ZigBee
  - EFR32xG22 running NCP Firmware for AoXPC

If these prerequisites are not fulfilled, follow the setup instructions in the
[Official Raspberry Pi User Guide](https://projects.raspberrypi.org/en/projects/raspberry-pi-setting-up),
and ensure a supported radio module is attached to your RPi4.

It also is required to add mosquitto PPA on the RPi4 to get mosquitto version which is above 2.0
to support MQTTv5:

``` sh
# Add mosquitto PPA to get mosquitto > 2.0 to support MQTTv5
curl -s http://repo.mosquitto.org/debian/mosquitto-repo.gpg.key | sudo apt-key add -
sudo curl -s  http://repo.mosquitto.org/debian/mosquitto-buster.list -o /etc/apt/sources.list.d/mosquitto-buster.list
sudo apt update
sudo apt install mosquitto mosquitto-clients
```

## Installation

The Unify distribution is targeted a Debian Buster platform. A binary debian
packages for a raspberry pi 4 are provided with the SDK and they are available
on github [release page](https://github.com/SiliconLabs/UnifySDK/releases). 

The simplest way to install the Unify components is to use the `apt` tool: 

```bash
sudo apt install uic-<component>_<version>_armhf.deb
```
 or for installing all components

 ```bash
sudo apt install ./uic-sdk-dir/*.deb
```

or for updating previously installed packages to a newer version

 ```bash
sudo apt reinstall ./uic-sdk-dir/*.deb
```

## Evaluation

Once all Unify Application components are installed and configured, one can
evaluate the system via running IoT service such as Unify dev_gui. Note that
the raspberry pi needs to be rebooted after installing the debian packages 
for the first time.

After a reboot the Unify dev_gui can be accessed from a browser on
[http://raspberrypi.local:3080](http://raspberrypi.local:3080).

_Note_ that the Unify user interface needs TCP access to the port 3080 and 1337
on the Raspberry Pi.

See the [Dev-GUI manual](applications/dev_ui/dev_gui/readme_user.md) for more
information about using this interface.
## Running Using Systemd

Normally, the Unify components should be run as a service. When the debian packages
are installed they automatically enables themselves as systemd services.

The the components can be started and stopped using the `systemctl` command (see systemctl
linux manual for details).

```bash
sudo systemctl start uic-<component-name>
sudo systemctl stop uic-<component-name>
```

To see the logs of the the components when it runs as a service, use the following command:

```bash
journalctl -u uic-<component-name>.service
```
## Configuring Unify Components
Configuration parameters for the unify components can be specified in two 
different ways. 

## Configuring logging

The logging of each unify component can be controlled by the two common 
configuration parameters, __log.level__ and __log.tag_level__-

When a component starts, the log is printed to stdout, as follows:

```
+ 2020-11-11 14:18:52:276827 <d> [sl_log] Setting log level to debug
+ 2020-11-11 14:18:52:277573 <d> [uic_component_fixtures] Completed: UIC Signal Handler
+ 2020-11-11 14:18:52:277631 <d> [mqtt_client] Constructing new mqtt_client instance.
+ 2020-11-11 14:18:52:277696 <d> [mqtt_wrapper_mosquitto] Initializing Mosquitto library.
```

The general log format is as follows:

```
<timestamp> <severity> [tag] <message>
```

The log level can be set globally and per tag.

For example:

```bash
zpc  --log.level i --log.tag_level uic_component_fixtures:d,mqtt_client:d
```

This will set the global log level to info but it enables the debug on the two
components such as the following: mqtt_client and uic_component_fixtures.

### How to configure logs rotation

The Unify services logs ends two places. First the journald system deamon
services the logs. In addition the logs are also dumped in the
`/var/log/syslog.log`. Journald will automatically rotate logs and not
overload the system with logs.

#### Configure Journalctl

Unify uses systemd-journald system for logging. It should be configured with `/etc/systemd/journald.conf` file.
This file contains several parameters, but we only need a few. For detailed description, please follow [man journald.conf(5)](https://man7.org/linux/man-pages/man5/journald.conf.5.html).
Also, you should know there are two logs storages in Linux: **persistent** and **volatile**. Settings for each of them are configured separatelly.
Parameters, started with `System` applies to persistent storage, located in `/var/log/`.
Parameters, started with `Runtime` applies to volatile storage, located in `/run/log/journal`.
| Parameter                             | Description                                                                                |
| --------------------------------------| ------------------------------------------------------------------------------------------ |
| SystemMaxUse, RuntimeMaxUse           | how much disk space the journal may use up at most                                         |
| SystemKeepFree, RuntimeKeepFree       | how much disk space journald shall leave free for other uses                               |
| SystemMaxFileSize, RuntimeMaxFileSize | how large individual journal files may grow at most                                        |
| SystemMaxFiles, RuntimeMaxFiles       | how many individual journal files to keep at most                                          |
| MaxFileSec                            | the maximum time to store entries in a single journal file before rotating to the next one |
| MaxRetentionSec                       | the maximum time to store journal files before deletion even if there is enough free space |

For disk space parameters (`SystemMaxUse`, `SystemKeepFree`, `SystemMaxFileSize`) specify values in bytes or use _K, M, G, T, P, E_ as multipliers.

For time parameters (`MaxFileSec`, `MaxRetentionSec`) specify values in seconds or use _"year", "month", "week", "day", "h"_ or _"m"_ as multipliers.
Set the parameter value to `0` to turn off this feature.

#### Configure log rotation syslog

If you have enabled debug logging of the Unify SDK packages. Chances are you
will quite fast load up some big log files in syslog. To mitigate this you
can change the log rotation in the syslog logrotation file
`/etc/logrotate.d/rsyslog`. Documentation on log rotation settings can be
found [here](https://manpages.debian.org/stretch/logrotate/logrotate.8.en.html).

## Setting up Mosquitto

The Unify framework need to the mosquitto MQTT broker to work. For this reason
mosquitto is a dependency of all the provided debian packages. The Unify
framework works with the default mosquitto installation but it may be
desireable to change the mosquito broker installation, eg. to enable TLS
support.

Configure the Mosquitto broker with the following parameters to enable
TLS support. Mosquitto can support TLS and non-TLS communication simultaneously
on two different ports. For example, in the following mosquitto.conf, Mosquitto Broker
will accept non-TLS communication on port 1883.

### mosquitto.conf

```bash
port 1883
allow_anonymous true
per_listener_settings true

listener 8883
cafile ca/ca.crt
certfile broker/broker.crt
keyfile broker/broker.key
require_certificate true
```
Having the config options: `"cafile" (client/ca.crt)`, `"certfile"
(broker/broker.crt)` and `"keyfile" (broker/broker.key)` in Mosquitto Broker
config file will make the Mosquitto Broker communicate with Unify components with TLS encryption.

For the purpose of testing a script `tls-certificates-generation.sh` has been
provided to create test certificates, for a TLS connection between the Unify components and Mosquitto Broker.

> WARNING: Don't use this this script in making production certificates

```bash
scripts/tls-certificates-generation.sh <broker IP/hostname>
```

_localhost_ may be provided when the MQTT broker and Unify Components are running on the same machine. The script will output the following files:

```
$ tree .
| broker
   | broker.crt
   | broker.key
| ca
   | ca.crt
   | ca.key
   | ca.srl
| client
    | client.crt
    | client.key
```

Each Unify component must use the _mqtt.cafile_, _mqtt.certfile_ and _mqtt.keyfile_ options
to enable the TLS encrypted connection. The option can be provided from command line or from the 
config file of the components.

>NOTE: the DevUI will need the certificates as input from the user interface itself.
