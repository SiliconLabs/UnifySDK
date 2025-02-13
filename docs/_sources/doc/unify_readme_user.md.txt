# Unify SDK User guide

This guide contains information which is common to all Unify applications.
For a having a general setup running, it assumes you have gone over the [Getting Started guide](getting_started_unify.md).
Details about each application can be found in the application specific user guides.

## Running Using Systemd

Normally, the Unify components should be run as a service. When the Debian packages
are installed they automatically enables themselves as systemd services.

the components can be started and stopped using the `systemctl` command (see systemctl
linux manual for details).

```bash
sudo systemctl start uic-<component-name>
sudo systemctl stop uic-<component-name>
```

To see the logs of the components when it runs as a service, use the following command:

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

``` bash
+ 2020-11-11 14:18:52:276827 <d> [sl_log] Setting log level to debug
+ 2020-11-11 14:18:52:277573 <d> [uic_component_fixtures] Completed: UIC Signal Handler
+ 2020-11-11 14:18:52:277631 <d> [mqtt_client] Constructing new mqtt_client instance.
+ 2020-11-11 14:18:52:277696 <d> [mqtt_wrapper_mosquitto] Initializing Mosquitto library.
```

The general log format is as follows:

```bash
<timestamp> <severity> [tag] <message>
```

The log level can be set globally and per tag.

For example:

```bash
eed --eed.datastore_file ./eed.db  --log.level i --log.tag_level uic_component_fixtures:d,mqtt_client:d
```

This will set the global log level to info but it enables the debug on the two
components such as the following: mqtt_client and uic_component_fixtures.

### How to configure logs rotation

The Unify services logs ends two places. First the journald system daemon
services the logs. In addition the logs are also dumped in the
`/var/log/syslog.log`. Journald will automatically rotate logs and not
overload the system with logs.

#### Configure Journalctl

Unify uses systemd-journald system for logging. It should be configured with `/etc/systemd/journald.conf` file.
This file contains several parameters, but we only need a few. For detailed description, please follow [man journald.conf(5)](https://man7.org/linux/man-pages/man5/journald.conf.5.html).
Also, you should know there are two logs storages in Linux: __persistent__ and __volatile__. Settings for each of them are configured separately.
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
will load up some big log files in syslog quite fast. To mitigate this you
can change the log rotation in the syslog log rotation file
`/etc/logrotate.d/rsyslog`. Documentation on log rotation settings can be
found [here](https://manpages.debian.org/stretch/logrotate/logrotate.8.en.html).

## Setting up Mosquitto

The Unify framework need to the mosquitto MQTT broker to work. For this reason
mosquitto is a dependency of all the provided Debian packages. The Unify
framework works with the default mosquitto installation but it may be
desireable to change the mosquito broker installation, eg. to enable TLS
support.

Configure the Mosquitto broker with the following parameters to enable
TLS support. Mosquitto can support TLS and non-TLS communication simultaneously
on two different ports. For example, in the following mosquitto.conf, Mosquitto Broker
will accept non-TLS communication on port 1883.

### mosquitto.conf

```bash
listener 1883
allow_anonymous true

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

```bash
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

### Using MQTT Explorer to Monitor Messages

You can publish MQTT message and observe the MQTT messages stored in the broker
with a [MQTT Explorer](https://mqtt-explorer.com/).
[MQTT Explorer](https://mqtt-explorer.com/) is an MQTT client that provides a
structured overview of your MQTT topics.

Configure [MQTT Explorer](https://mqtt-explorer.com/) to connect to the host
where your MQTT broker is located and the topics can be observed. Use the
[Unify Specifications](./unify_specifications/index.rst)
if you need assistance to understand what topics and they data represent.

## Evaluation

Once all Unify Application components are installed and configured, one can
evaluate the system via running IoT service such as Unify dev_gui.

The Unify dev_gui can be accessed from a browser on
[http://raspberrypi.local:3080](http://raspberrypi.local:3080).

_Note_ that the Unify user interface needs TCP access to the port 3080 and 1337
on the Raspberry Pi.

See the [Dev-GUI manual](../applications/dev_ui/dev_gui/readme_user.md) for more
information about using this interface.
