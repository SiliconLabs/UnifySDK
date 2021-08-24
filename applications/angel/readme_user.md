# ANGEL User's Guide

The group manager MQTT Client, ANGEL, is an application that manages the by-group
topic `ucl/by-group/`. It acts as a lookup entity that provides protocol controllers
 or other MQTT clients information about the interactions between
 groups, endpoints, and supported commands. It does not handle the topics that are
 used to execute group commands, for example
 `ucl/by-group/<group-id>/<Command>`, which is done by protocol controllers.

The ANGEL application does not persist any collected information. Instead, it collects
and publishes its data over the MQTT Framework.

Protocol controllers can run without the ANGEL application, which results in limited
functionality.

## Running the ANGEL

### Using Systemd Service

The best way to run the ANGEL is using the Systemd service that is installed with
the Debian installer. For more information, see [README.md](../../README.md).

### Using Command Line

Alternatively, the ANGEL may be run by executing `uic-angel`. It is possible to
configure the MQTT server, database path, through command line options. For more
details, run `uic-angel --help`.

### Output Examples

```
pi@rpi4ovdo1:~/demo $ ./uic-angel --help

Usage: /usr/bin/uic-angel [Options]

Options:
  --conf arg (=/etc/uic/uic.cfg)        Config file in YAML format
  --help                                Print this help message and quit
  --dump-config                         Dumps the current configuration on a
                                        YAML config file format that can be
                                        passed to --conf option
  --version                             Print version information and quit

Following options can also be in a Config file.
 Options and values passed on command line here take precedence over the options and values in config file:
  --mqtt.host arg (=localhost)          MQTT broker hostname or IP
  --mqtt.port arg (=1883)               MQTT broker port
  --log.level arg (=d)                  Log Level (d,i,w,e,c)
  --log.tag_level arg                   Tag based log level
                                        Format: <tag>:<severity>,
                                        <tag>:<severity>, ...
  --mqtt.client_id arg (=uic-angel)     override mqtt client name
```

```
pi@raspberrypi:~ $ /usr/bin/uic-angel
2021-06-04 13:53:21:059070 <d> [sl_log] Setting log level to debug
2021-06-04 13:53:21:069203 <i> [mqtt_client] connecting to "localhost" port:1883
2021-06-04 13:53:21:073466 <i> [mqtt_client] subscribing to "ucl/by-unid/+/+/Groups/Attributes/GroupList/Reported" qos:0
2021-06-04 13:53:21:074232 <i> [mqtt_client] subscribing to "ucl/by-unid/+/+/Groups/Attributes/+/Name/Reported" qos:0
2021-06-04 13:53:21:074661 <i> [mqtt_client] subscribing to "ucl/by-unid/+/+/+/SupportedCommands" qos:0
2021-06-04 13:53:21:075888 <i> [mqtt_client] publishing "ucl/by-group/1/GroupName" payload:{"value":"Lights"} ret:true

```




