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

## Developing protocol controllers

See [How to Develop a Protocol Controller](how_to_develop_a_protocol_controller.rst) 

The attribute mapper helps to translate protocol commands to UCL MQTT messages.
See [How to write UAM files](how_to_write_uam_files.md)

## Extend UCL clusters

See [How to write a new Cluster](how_to_write_new_cluster.md) 

It might be needed for non-zigbee controllers if some description are missing.


## Overview of relations among Unify Applications

The Unify Framework consists of several applications including Protocol Controllers, IoT
Services (e.g. Developer GUI, UPTICap) and multiple application services that
facilitate various functionalities such as SmartStart Provisioning, Group
Management, Name and Location Service and OTA Image Provider.

The following table presents the relationship amongst Unify Framework applications and
the Unify Protocol Controllers. If a given component is supported via a
Protocol Controller, it will be marked as [x].

|                                                                             | [ZPC](../applications/zpc/readme_user.md) | [ZigPC](../applications/zigpc/readme_user.md) | [AoXPC](../applications/aox/applications/aoxpc/readme_user.md) |
| --------------------------------------------------------------------------- | :------------------------------------- | :----------------------------------------- | :---------------------------------------------------------- |
| [UPVL](../applications/upvl/readme_user.md)                                    | [x]                                    | [x]                                        |                                                             |
| [GMS](../applications/gms/readme_user.md)                                      | [x]                                    | [x]                                        |                                                             |
| [NAL](../applications/nal/readme_user.md)                                      | [x]                                    | [x]                                        | [x]                                                         |
| [OTA Image Provider](../applications/image_provider/readme_user.md)            | [x]                                    | [x]                                        |                                                             |
| [Dev GUI](../applications/dev_ui/dev_gui/readme_user.md)                       | [x]                                    | [x]                                        | [x]                                                         |
| [UPTICap](../applications/upti_cap/readme_user.md)                             | [x]                                    | [x]                                        |                                                             |
| [UPTIWriter](../applications/upti_writer/readme_user.md)                       | [x]                                    | [x]                                        |                                                             |
| [AoX Positioning](../applications/aox/applications/positioning/readme_user.md) |                                        |                                            | [x]                                                         |
