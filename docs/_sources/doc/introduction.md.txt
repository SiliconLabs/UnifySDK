# Unify Overview

```{toctree}
---
maxdepth: 2
hidden:
titlesonly:
---
self
./running_unify.md
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



This documentation is for the [latest release of the Unify Host SDK](https://github.com/SiliconLabs/UnifySDK/releases/latest).

* The Silicon Labs Unify Host SDK source code may be found at
  <https://github.com/SiliconLabs/UnifySDK>
* Binary packages for Raspberry Pi 4 (Raspbian 11, Debian Bullseye 64-bit) can be found
  here <https://github.com/SiliconLabs/UnifySDK/releases>

**To demo Unify on a Raspberry Pi see** [Running Unify on a Raspberry Pi](running_unify.md)

**To get started developing with Unify** see [Getting Started as a Developer](getting_started_as_developer.md)

**For detailed information about the Unify Framework** see [Unify Framework](UnifySDK.md)

**For detailed information about a specific protocol** see [Protocol Controllers](protocol_controllers.md)

## Overview

Unify SDK is a software framework that simplifies the developer experience, removing difficult parts of network control and network management as it relates to gateway and hub development in IoT applications. It can also provide Matter bridge functionality to other protocols that do not natively run matter.

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

The Unify Host SDK contains:

* The Unify Framework - A powerful IoT gateway framework that can support multiple wireless protocols.
* Protocol Controllers that implement various wireless protocols.
* Multiprotocol Host Software - A collection of host software for simultaneously
  running multiple protocol stacks on the host.
  Using a single radio co-processor, you can run Zigbee, OpenThread, and Bluetooth.

Here is a component view along with further detailed descriptions below.

```{eval-rst}
.. figure:: assets/img/unify-sdk-block-diagram.jpg
   :alt: Unify Host SDK
   :width: 75%
   :align: center
```

## Unify Framework

The Unify Framework has support for:

  [![Z-Wave](assets/img/Z-Wave_logo.png)](../applications/zpc/readme_user.md)
  [![Zigbee](assets/img/Zigbee_logo.png)](../applications/zigpc/readme_user.md)
  [![Matter](assets/img/matter_logo.png)](https://siliconlabs.github.io/matter/latest/unify/readme_user.html)
  [![Bluetooth AoX](assets/img/Bluetooth_logo.png)](../applications/aox/applications/aoxpc/readme_user.md)

Further details can be found on the [Unify Framework introduction page](UnifySDK.md).

## Multiprotocol 

The Unify Host SDK contains support for running Zigbee, OpenThread, and Bluetooth on a
Linux host, using only a single low-cost Radio Co-Processor (RCP) chip from Silicon Labs.
This system allows for running multiple protocols simultaneously using the same RCP.

Further details can be found on the [Multiprotocol Host Software page](multiprotocol.md).
