# UPVL User's Guide

The _Unify SDK Provisioning List_ (UPVL) MQTT Client is an
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

## Running the UPVL

### Using Systemd Service

The best way to run the UPVL is using the Systemd service that is installed with
the Debian installer. For more information, see [README.md](../../README.md).

### Using Command Line

Alternatively, the UPVL may be run by executing `uic-upvl`. It is possible to
configure the MQTT server, database path, through command line options. For more
details about the options, run `uic-upvl --help`.

### Output Examples

```
pi@rpi4ovdo1:~/demo $ ./uic-upvl --help

Usage: uic-upvl [Options]

Options:
  --conf arg (=/etc/uic/uic.cfg)        Config file in YAML format
  --help                                Print this help message and quit
  --sample_conf_file                    Print sample YAML config file that can
                                        be passed to --conf option
  --version                             Print version information and quit

Following options can also be in a Config file.
 Options and values passed on command line here take precedence over the options and values in config file:
  --mqtt.host arg (=localhost)          MQTT broker hostname or IP
  --mqtt.port arg (=1883)               MQTT broker port
  --mqtt.cafile arg                     Path to file containing the PEM encoded
                                        CA certificate to connect to Mosquitto
                                        MQTT broker for TLS encryption
  --mqtt.certfile arg                   Path to file containing the PEM encoded
                                        client certificate to connect to
                                        Mosquitto MQTT broker for TLS
                                        encryption
  --mqtt.keyfile arg                    Path to file containing the PEM encoded
                                        CA certificate to connect to Mosquitto
                                        MQTT broker for TLS encryption
  --log.level arg (=d)                  Log Level (d,i,w,e,c)
  --log.tag_level arg                   Tag based log level
                                        Format: <tag>:<severity>,
                                        <tag>:<severity>, ...
  --datastore.file arg (=/var/lib/uic/database.db)
                                        Datastore database file
  --mqtt.client_id arg (=upvl_client)   Client ID for MQTT client
  --upvl.db_file arg (=upvl_test.db)    File name of UPVL database
```

```
pi@rpi4ovdo1:~/demo $ ./uic-upvl --conf ./upvl.conf
MQTT broker from config 192.168.1.209
MQTT port number from config 1883
UPVL DB filename from config upvl_database.db
Logging level from config c,i,e
2021-02-10 18:19:52:301949 <i> [upvl_db] Using database file upvl_database.db
2021-02-10 18:19:52:475877 <i> [upvl_mqtt] Connected to broker on 192.168.1.209 port 1883
2021-02-10 18:19:52:476183 <i> [upvl_db] To publish: []
2021-02-10 18:19:52:477271 <i> [upvl_mqtt] (Re-)connected to broker
2021-02-10 18:19:52:477344 <i> [upvl_mqtt] Subscribing
2021-02-10 18:21:36:284275 <i> [upvl_mqtt] Subscriber: Received upvl topic "ucl/SmartStart/List/Update"
2021-02-10 18:21:36:297850 <i> [upvl_db] Updated DB for DSK 20434-14882-53263-05030-45268-50517-20518-60286.
2021-02-10 18:21:36:298437 <i> [upvl_db] To publish: [{"DSK":"20434-14882-53263-05030-45268-50517-20518-60286","Include":true,"ProtocolControllerUnid":"","Unid":""}]
2021-02-10 18:21:36:298810 <i> [upvl_mqtt] Publisher: Published message with id: 4
2021-02-10 18:21:57:483021 <i> [upvl_mqtt] Subscriber: Received upvl topic "ucl/SmartStart/List/Update"
2021-02-10 18:21:57:496147 <i> [upvl_db] Updated DB for DSK 20434-14882-53263-05030-45268-50517-20518-60287.
2021-02-10 18:21:57:496719 <i> [upvl_db] To publish: [{"DSK":"20434-14882-53263-05030-45268-50517-20518-60286","Include":true,"ProtocolControllerUnid":"","Unid":""},{"DSK":"20434-14882-53263-05030-45268-50517-20518-60287","Include":true,"ProtocolControllerUnid":"","Unid":""}]
2021-02-10 18:21:57:497101 <i> [upvl_mqtt] Publisher: Published message with id: 5
2021-02-10 18:22:57:815468 <i> [upvl_mqtt] Subscriber: Received upvl topic "ucl/SmartStart/List/Remove"
2021-02-10 18:22:57:815696 <E> [upvl_mqtt] Cannot convert payload to json, error trailing comma at line 3 column 1
2021-02-10 18:23:21:914602 <i> [upvl_mqtt] Subscriber: Received upvl topic "ucl/SmartStart/List/Remove"
2021-02-10 18:23:21:928011 <i> [upvl_db] Removed 1 entries with DSK 20434-14882-53263-05030-45268-50517-20518-60287.
2021-02-10 18:23:21:928563 <i> [upvl_db] To publish: [{"DSK":"20434-14882-53263-05030-45268-50517-20518-60286","Include":true,"ProtocolControllerUnid":"","Unid":""}]
2021-02-10 18:23:21:929022 <i> [upvl_mqtt] Publisher: Published message with id: 6
2021-02-10 18:23:31:265982 <i> [upvl_mqtt] Subscriber: Received upvl topic "ucl/SmartStart/List/Remove"
2021-02-10 18:23:31:278848 <i> [upvl_db] Removed 1 entries with DSK 20434-14882-53263-05030-45268-50517-20518-60286.
2021-02-10 18:23:31:279392 <i> [upvl_db] To publish: []
2021-02-10 18:23:31:279759 <i> [upvl_mqtt] Publisher: Published message with id: 7

```




