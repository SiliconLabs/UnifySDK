# Unify Overview

```{toctree}
---
maxdepth: 2
hidden:
titlesonly:
---
self
./getting_started_unify.md
```

```{toctree}
---
maxdepth: 2
hidden:
---
./UnifySDK.md
./unify_framework_services.md
```

```{toctree}
---
maxdepth: 3
hidden:
---
./protocol_controllers.md
./unify_iot_services.md
```

```{toctree}
---
maxdepth: 3
hidden:
---
./getting_started_as_developer.md
```



```{toctree}
---
maxdepth: 2
hidden:
titlesonly:
---
Unify Specifications <unify_specifications/index.rst>
./system_requirements.md
standards/known-abbreviations.md
./license.md
```



This documentation is for the [latest release of the Unify SDK](https://github.com/SiliconLabs/UnifySDK/releases/latest).

* The Silicon Labs Unify SDK source code may be found at
  <https://github.com/SiliconLabs/UnifySDK>
* Binary packages for Raspberry Pi 4 (Raspbian 12, Debian Bookworm 64-bit) can be found
  here <https://github.com/SiliconLabs/UnifySDK/releases>

**Use [Portable Runtime](portable_runtime/readme_user.md) for a quick demo with Unify**

## Overview

Unify is a software framework that simplifies the developer experience, removing difficult parts of network control and network management as it relates to gateway and hub development in IoT applications. Unify SDK natively supports Z-Wave protocol, and also provides an example code to showcase Matter bridge functionality to Z-Wave and Custom protocols that do not run Matter.

Unify SDK is developed as an open, modular, and portable architecture based on ubiquitous lightweight MQTT technology. The software framework is designed and developed on Linux. It provides an extensible set of software features to enable IoT wireless connectivity in gateways, multiprotocol scalability and application processor-based end products. Unify SDK simplifies development and ongoing software maintenance for IoT gateways and application processor-based end devices.

The Unify SDK provides software source and binary packages for Raspberry Pi 4. 

Unify SDK provides common building blocks that ease connectivity across IoT ecosystems.

When developing IoT products that leverage a Linux operating system and a
Silicon Labs radio, such as an IoT gateway or an advanced IoT end device,
additional software is needed to interface with the IoT radio and provide
interoperability with the host application. To equip customers developing
complex IoT applications for single or multi-protocol use cases, Silicon Labs
has created a single repository for important customer software needs on a Linux
gateway. We've aggregated software from multiple repositories and simplified the
distribution to enhance ease of use and accessibility.

The Unify Ecosystem contains:

* The Unify Framework - A powerful IoT gateway framework that can support multiple wireless protocols.
* A Z-Wave protocol Controller that implement wireless Z-Wave protocol.
* Current software solution can also be leveraged to use proprietary(custom) Protocol Controllers.
* Emulation of end device for better dev experience.

**Refer [how to develop Protocol Controller](how_to_develop_a_protocol_controller.rst) for adding proprietary(custom) Protocol Contoller to Unify Ecosystem**

Here is a component view along with further detailed descriptions below.

```{eval-rst}
.. figure:: assets/img/unify-ecosystem-block-diagram.jpg
   :alt: Unify Ecosystem
   :width: 75%
   :align: center
```

## Unify Framework

The Unify Framework has support for:

  [![Z-Wave](assets/img/Z-Wave_logo.png)](https://siliconlabssoftware.github.io/z-wave-protocol-controller)
  [![Matter](assets/img/matter_logo.png)]

Further details can be found on the [Unify Framework introduction page](UnifySDK.md).

## Emulated End Device (EED)
The EED emulates a single or set of end devices as if controlled by a protocol controller. This is solely for better developer experience and is a experimental implementation.

Further details can be found on the [EED User Guide](../applications/examples/applications/emulated_end_device/readme_user.md).
