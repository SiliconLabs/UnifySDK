# Unify Host SDK

```{toctree}
---
maxdepth: 2
hidden:
---
self
./UnifySDK.md
./multiprotocol.md
./getting_started.md
./readme_developer.md
./system_requirements.md
./qr_code_scanner.md
./license.md
```

This documentation is for the [latest release of the Unify Host SDK](https://github.com/SiliconLabs/UnifySDK/releases/latest).

The Unify Host SDK contains non-embedded applications developed by Silicon Labs.

It is the main source for Silicon Labs customers who are working with Linux
based applications in conjunction with Silicon Labs products.

* The Silicon Labs Unify Host SDK source code may be found at
  <https://github.com/SiliconLabs/UnifySDK>
* Released packages for Raspberry Pi 4 (Raspbian 11, Debian Bullseye 64-bit) can be found
  here <https://github.com/SiliconLabs/UnifySDK/releases>

## Introduction

When developing IoT products that leverage a Linux operating system and a
Silicon Labs radio, such as an IoT gateway or an advanced IoT end device,
additional software is needed to interface with the IoT radio and provide
interoperability with the host application. To equip customers developing
complex IoT applications for single or multi-protocol use cases, Silicon Labs
has created a single repository for important customer software needs on a Linux
gateway. We've aggregated software from multiple repositories and simplified the
distribution to enhance ease of use and accessibility.

The Unify Host SDK contains two main software bundles:

* The Unify Framework - A powerful IoT gateway framework that supports multiple wireless protocols.
* Multiprotocol Host Software - A collection of host software for simultaneously
  running multiple protocol stacks on the host.
  Using a single radio co-processor, you can run Zigbee, OpenThread, and Bluetooth.

Here is a component view along with further detailed descriptions below.

```{eval-rst}
.. figure:: assets/img/unify_host_sdk.png
   :alt: Unify Host SDK
   :width: 75%
   :align: center
```

## Unify Framework

The Unify Framework simplifies the developer experience, removing difficult
parts of network control and network management as it relates to gateway & hub
development in IoT applications.

It provides an extensible set of software features to enable IoT wireless
connectivity in gateways, multiprotocol scalability, and application
processor-based end products. Unify Framework simplifies development and ongoing
software maintenance for IoT gateways and application processor-based end
devices.

The Unify Framework has support for:

  [![Bluetooth AoX](assets/img/Bluetooth_logo.png)](../applications/aox/applications/aoxpc/readme_user.md)
  [![Zigbee](assets/img/Zigbee_logo.png)](../applications/zigpc/readme_user.md)
  [![Z-Wave](assets/img/Z-Wave_logo.png)](../applications/zpc/readme_user.md)
  [![Matter](assets/img/matter_logo.png)](https://siliconlabs.github.io/matter/latest/unify/readme_user.html)

Further details can be found on the [Unify Framework introduction page](UnifySDK.md).

## Multiprotocol Host Software

The Unify Host SDK contains support for running Zigbee, OpenThread, and Bluetooth on a
Linux host, using only a low-cost Radio Co-Processor (RCP) chip from Silicon Labs.
This system allows for running multiple protocols simultaneously using the same RCP.

Further details can be found on the [Multiprotocol Host Software page](multiprotocol.md).
