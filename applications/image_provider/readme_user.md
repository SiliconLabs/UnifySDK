# Image Provider User's Guide

The Image Provider is a generic application that allows storing and
distributing OTA Firmware images for Protocol Controllers over MQTT.

For more details about the interactions between the Image Provider and Protocol
Controllers, see the Unify specifications - Common OTA FW Update Service
chapter.

The Image Provider application is required for most Protocol Controller to
access firmware images for a given node. Protocol Controllers running without
the Image Provider application available will most likely not be able to provide
OTA functionalities.

## Running the Image Provider

### Using Systemd Service

The best way to run the Image Provider is using the Systemd service that is
installed with the Debian installer.
For more information, see [README.md](../../README.md).

### Using the Command Line

Alternatively, you can run the Image Provider by executing `uic-image-provider`.
You can configure the MQTT server, database path, through command line options.
For details about options, run the following command:

```bash
pi@unify:uic-image-provider --help
```

Make sure that you do not run the Image Provider both as a service and using the
command line.

```bash
pi@unify:~ $ service uic-image-provider status
● uic-image-provider.service - Unified IoT OTA Image Provider
   Loaded: loaded (/lib/systemd/system/uic-image-provider.service; enabled; vendor preset: enabled)
   Active: active (running) since Tue 2021-08-17 14:50:26 CEST; 1 day 20h ago
 Main PID: 13235 (uic-image-provi)
    Tasks: 2 (limit: 4915)
   CGroup: /system.slice/uic-image-provider.service
           └─13235 /usr/bin/uic-image-provider
```

If the service is running, stop it using the following command before
you run the application manually:

```bash
pi@unify:~ $ service uic-image-provider stop
```

## Using the Image Provider

### OTA Folder Structure

The Image Provider will use the configured **image_provider.image_path** to
store a JSON file that will represent the list of available images.

The *images.json* file must be configured to indicate which images to advertise
to the MQTT broker/Protocol Controllers.

```bash
pi@unify:/var/lib/uic-image-provider $ tree
.
├── images.json
└── updates
    ├── ZW_SensorPir_7.15.4_256_ZGM130S_REGION_EU.gbl
    └── ZW_SwitchOnOff_7.15.4_256_ZGM130S_REGION_EU.gbl
```

### Metadata File Name and Format

The filename for the metadata is static and must stay *images.json*.

The Image Provider will poll the images.json for changes with a configurable
period. It will then advertise images.

The JSON file must follow the following JSON schema:

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "Image Provider Metadata file format",
  "description": "List of Available Firmwares files for the Image Provider",
  "definitions": {
    "Image Entry": {
      "type": "object",
      "properties": {
        "FileName": {
          "type": "string"
        },
        "Uiid": {
          "type": "string"
        },
        "Unid": {
          "type": "array",
          "items": {
            "type": "string"
          }
        },
        "Version": {
          "type": "string"
        },
        "ApplyAfter": {
          "type": "string"
        },
        "Md5": {
          "type": "string"
        }
      },
      "required": [
        "FileName",
        "Uiid",
        "Unid",
        "Version",
        "ApplyAfter",
        "Md5"
      ]
    }
  },
  "type": "object",
  "properties": {
    "Version": {
      "type": "string"
    },
    "Images": {
      "type": "array",
      "items": {
        "$ref": "#/definitions/Image Entry"
      }
    }
  },
  "required": [
    "Version",
    "Images"
  ]
}
```

This is a sample *images.json* file advertising 2 images.

```json
{
  "Version": "1",
  "Images": [
   {
      "FileName": "updates/ZW_SensorPir_7.15.4_256_ZGM130S_REGION_EU.gbl",
      "Uiid": "ZWave-0000-0003-0004-00-01",
      "Unid": [],
      "Version": "10.15.4",
      "ApplyAfter": "2021-06-29T16:39:57+02:00",
      "Md5": "cZYo9OIM+rnuUk6G9dXmtg=="
    },
    {
      "FileName": "updates/ZW_SwitchOnOff_7.15.4_256_ZGM130S_REGION_EU.gbl",
      "Uiid": "ZWave-0000-0002-0004-00-01",
      "Unid": ["zw-C87E6FB8-0006", "zw-C87E6FB8-0007", "zw-C87E6FB8-0008"],
      "Version": "10.15.4",
      "ApplyAfter": "2021-06-29T16:42:59+02:00",
      "Md5": "GnzJgyfhQhzJQHRVtVy8vw=="
    }
  ]
}
```

The following parameters must be provided for each Firmware entry:

* **FileName** : This is the relative path from the *images.json* to the
firmware files. Absolute path can be used.
* **Uiid** : The Unique Image Identifier. This allows Protocol Controller
to identify know which nodes can receive the firmware. More details about how
to build the UIID is described in each individual Protocol Controller manual
* **Unid** : A list of UNIDs that should receive the update. An empty array
will instruct the Protocol Controller to update all compatible nodes. An array
with UNIDs will instruct the Protocol Controller to update only the list of
nodes in this array.
* **Version** : The version of the firmware file. It may be used by
protocol controllers to prevent a downgrade or attempting to upgrade to the same
version. The construct and interpretation of the version string is
Protocol Controller specific
* **ApplyAfter** : A timestamp encoded using RFC 3339, that will indicate when
to first attempt to apply the Firmware Update. A date in the past will
instruct Protocol Controllers to initiate the firmware update as soon as
possible.
* **Md5** : A checksum that will verify that the data located in the *fileName*
is coherent with what we expect.


#### Md5 Checksum calculation

The Md5 checksum value must be set to a base64 encoded md5 digest of image file.

In Rust, the checksum can be generated like this:

```rust
base64::encode(md5::compute(buffer).to_vec());
```

The checksum string can also be created using openssl:

```bash
cat ZW_SensorPir_7.15.4_256_ZGM130S_REGION_EU.gbl | openssl dgst -binary -md5 | openssl base64
```

### Output Examples

```bash
pi@unify:~ $ uic-image-provider --help

Usage: uic-image-provider [Options]

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
  --log.level arg (=d)                  Log Level (d,i,w,e,c)
  --log.tag_level arg                   Tag based log level
                                        Format: <tag>:<severity>,
                                        <tag>:<severity>, ...
  --datastore.file arg (=/var/lib/uic/database.db)
                                        Datastore database file
  --mqtt.client_id arg (=uic-image-provider)
                                        override mqtt client name
  --provider.image_path arg (=/var/lib/uic-image-provider)
                                        image path
  --provider.poll_period arg (=15)      poll period

```

```bash
pi@unify:~ $ uic-image-provider
2021-07-07 16:37:37:790401 <d> [sl_log] Setting log level to debug
2021-07-07 16:37:37:799641 <i> [mqtt_client] connecting to "localhost" port:1883
2021-07-07 16:37:37:802863 <i> [image_watcher] Image storage path /var/lib/uic-image-provider
2021-07-07 16:37:37:803377 <i> [mqtt_client] subscribing to "ucl/OTA/data/+/+/get" qos:0
2021-07-07 16:37:37:805596 <i> [image_watcher] Publishing image announce to "ucl/OTA/info/ZWave-000-002-003_EU/all"
2021-07-07 16:37:37:806087 <i> [image_watcher] Publishing image announce to "ucl/OTA/info/ZWave-002-002-003_EU/zw-C87E6FB8-0006"
2021-07-07 16:37:37:806471 <i> [image_watcher] Publishing image announce to "ucl/OTA/info/ZWave-002-002-003_EU/zw-C87E6FB8-0007"
2021-07-07 16:37:37:806863 <i> [image_watcher] Publishing image announce to "ucl/OTA/info/ZWave-001-002-003_EU/zw-C87E6FB8-0005"
2021-07-07 16:37:41:296142 <i> [image_watcher] Image requested for ZWave-002-002-003_EU/zw-C87E6FB8-0006
2021-07-07 16:37:41:297280 <i> [mqtt_handler] Publishing binary payload to "ucl/OTA/data/ZWave-002-002-003_EU/zw-C87E6FB8-0006"

```




