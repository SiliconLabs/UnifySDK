# Getting Started as a Developer

```{toctree}
---
maxdepth: 2
hidden:
---
./readme_developer.md
```

The [Unify Developer Guide](readme_developer.md) contains a lot of useful information for Unify developers. 

The [Unify Build Guide](readme_building.md) is the best starting point for working with the source code.

The [Unify User Guide](unify_readme_user.md) contains helpful tips on debugging within Unify.

## Development Platform Recommendations

The Unify reference platform is a Raspberry Pi 4 however it is understood that users may not wish to run on a Raspberry Pi in a final product. Porting to Debian based linux should be straightforward.

System requirements for flash, RAM, and dependencies are listed at [here](system_requirements.md)

## Developing IoT Services

The IoT or cloud connector must use UCL (MQTT) to communicate with the other components of Unify. See [How to Develop an IoT service](how_to_develop_an_iot_service.md) Unify uses MQTT to communicate among each component using the Mosquitto MQTT broker. UCL is the format of the MQTT messages. The [Unify Framework Specification](./unify_specifications/index.rst) contains the definitions for UCL.

Also see <a href="../reference_ucl_mqtt/index.html">UCL MQTT API</a>

## Developing EED
See [Building EED](../applications/examples/applications/emulated_end_device/readme_developer.md) on how to build EED. The section also has some useful notes for developers interested in updating EED.

## Developing protocol controllers

See [How to Develop a Protocol Controller](how_to_develop_a_protocol_controller.rst) 

The attribute mapper helps to translate protocol commands to UCL MQTT messages.
See [How to write UAM files](how_to_write_uam_files.md)

## Extend UCL clusters to non-zigbee controllers

Refer related chapter in [Z-Wave-Protocol-Controller documentation](https://siliconlabssoftware.github.io/z-wave-protocol-controller)
 
