# Python helper CTT script dev documentation

This documentation explain the choices used in the project and how to contribute to it.

## Project structure

The project structure can be found bellow :

```bash 
├── command_classes
│   ├── __init__.py
│   ├── clusters
│   │   ├── __init__.py
│   │   ├── level.py
│   │   ├── on_off.py
│   │   └── ....
│   ├── config
│   │   ├── __init__.py
│   │   └── unify_config.py
│   ├── mqtt
│   │   ├── __init__.py
│   │   └── mqtt_manager.py
│   ├── utils
│   │   ├── __init__.py
│   │   ├── pretty_print.py
│   │   └── utils.py
│   ├── sound_switch.py
│   └── ....
├── config.ini
└── requirements.txt
```

> NOTE : `__init_.py` are empty and needed do not remove them. They are used to identify python packages.

| Name               | Description                                                    |
| ------------------ | -------------------------------------------------------------- |
| `cluster/`         | Helpers to send appropriate commands to ZCL clusters           |
| `config/`          | Handle parsing of config.ini                                   |
| `mqtt/`            | MQTT manager : Handle connection, subscribe and publish events |
| `utils/`           | Misc files : print helper, ...                                 |
| `command_classes/` | Contains specific CTT test for associated command              |
| `config.ini`       | Config file (see user guide)                                   |
| `requirements.txt` | Python dependencies                                            |

## Third parties

We try to reduce the third parties to the absolutely needed ones. Try to not add third parties unless absolutely needed.

| Name        | Description                         |
| ----------- | ----------------------------------- |
| `paho-mqtt` | Handle MQTT connection to Mosquitto |
| `colorist`  | Helper to make text more readable   |

## Write a CTT test script

Each CTT test script should import :

```python
import mqtt.mqtt_manager as mqtt_manager
import utils.pretty_print as display

from clusters import on_off, level
```

| Name                 | Description                        |
| -------------------- | ---------------------------------- |
| `mqtt.mqtt_manager`  | Initialize MQTT environnement      |
| `utils.pretty_print` | Helpers to make text more readable |
| `clusters `          | The ZCL clusters needed            |


In the main function of your script you should start with `add_node()` and end with `remove_node()`. Those function will handle the flow of adding the new CTT device and remove it when we are done.

`add_node()` is the more important one since it will also find the Uuid of the newly included CTT device.

```python

if __name__ == '__main__':
    mqtt_manager.add_node()

    mqtt_manager.remove_node()
```

Then you should define top-level function that describe what the script is doing. It should use `display.action_description` to inform the user of what's its doing. Then it should call the corresponding command in the cluster. 

If the supported command/cluster is missing [you'll need to implement it](#cluster-support).


```python
def change_default_tone(tone_id):
    display.action_description(f"Changing default tone ID to {tone_id}")
    level.command_move_frequency(tone_id)
```

Your script should look like this at the end : 

```python
import mqtt.mqtt_manager as mqtt_manager
import utils.pretty_print as display

from clusters import on_off, level

def change_default_tone(tone_id):
    display.action_description(f"Changing default tone ID to {tone_id}")
    level.command_move_frequency(tone_id)

def change_default_volume(volume):
    display.action_description(f"Changing default volume to {volume}")
    level.command_move_level(volume)

def play_tone(tone_id):
    display.action_description(f"Play tone {tone_id}")
    on_off.write_attributes_on_time(tone_id)

def play_default_tone():
    display.action_description(f"Play default tone")
    on_off.command_on()

def stop_tone():
    display.action_description(f"Stop tone")
    on_off.command_off()

if __name__ == '__main__':
    mqtt_manager.add_node()

    change_default_tone(2)
    change_default_volume(100)
    change_default_volume(0)
    play_tone(3)
    stop_tone()
    play_default_tone()

    mqtt_manager.remove_node()
```

This structure makes the script easy to understand and hide unimportant layers.

## Cluster support

You might need to add some commands/cluster for your CTT script. This can be done in the `applications/zpc/scripts/certification/command_classes/clusters` package. 

Start by including the mqtt_manager :

```py
import mqtt.mqtt_manager as mqtt_manager
```

This way you can access to the `send_unify_cluster_command` that will send the commands to the corresponding unify cluster. 

> NOTE : this function will work only if you called `add_node()` beforehand in your CTT script. The device Uuid is inferred from this function that's why we don't need to specify it. You can also set it manually if needed in `zwave_ctt_end_device_node` (`mqtt_manager.py`).

Then define the cluster name that will be used in the script : 

```py
CLUSTER_NAME = "OnOff"
```

You can then implement a cluster command easily : 

```py
def write_attributes_on_time(value):
    mqtt_manager.send_unify_cluster_command(CLUSTER_NAME, "WriteAttributes", '{"OnTime":%d}' % value)
```

> NOTE : `send_unify_cluster_command` handles the display of the topic that will be published no need to print anything here.