# Unify Host SDK

```{toctree}
---
maxdepth: 2
hidden:
numbered:
---
Unify Host SDK <HostSDK.md>
Unify SDK <UnifySDK.md>
Unify SDK Library <unify_library_overview.md>
```


Welcome to the Silicon Labs Unify Host SDK.

This SDK contains non-embedded applications developed by Silicon Labs. The
Host SDK is the main source for Silicon Labs customers who are working with
Linux based applications in conjunction with Silicon Labs products.

The Silicon Labs Unify Host SDK source code may be found at
https://github.com/SiliconLabs/UnifySDK and released packages for Raspberry Pi 4
(Raspbian 10,Debian Buster) can be found here
https://github.com/SiliconLabs/UnifySDK/releases

## Introduction

When developing IoT products which targets a Linux system and a Silicon Labs radio,
such as an IoT gateway or an advanced IoT end device, there is a need 
for software to interface with the radio to perform the heavy lifting
of the IoT protocol. (**** this paragraph is not good please help :-) ****)

The Unify Host SDK provides several categories of software.

**** ADD FIGURE HERE *****

### A communication daemon
(Matteo please fill in here)

The Co-Processor Communication (CPC) enables one host system to communicate with a
Network co-processor device (NCP), also named the secondary device or secondary,
by physical transport (UART, SPI, and so on). In CPC, data transfers between
processors are segmented in sequential packets. Transfers are guaranteed to be
error-free and sent in order. Multiple applications can send or receive on the
same endpoint without worrying about collisions. A CPC daemon (CPCd) is provided
to allow applications on Linux to interact with a secondary running CPC

### IoT protocol stacks

Enables full IoT protocol support on SiLabs Radio Co processor chips, which 
does not have the memory capacity to run the full protocol stacks locally. Using
the CPC multiple protocol stacks can use the same physical radio.

Currently the following protocols are supported
- Zigbee ([zigbeed])
- OpenThread ([OTBR])
- Bluetooth ([cpc-hci-bridge])

### Unify SDK

The Unify SDK, a software framework, simplifies the developer experience, removing
difficult parts of network control and network management as it relates to
gateway & hub development in IoT applications.

It provides an extensible set of software features to enable IoT wireless
connectivity in gateways, multi protocol scalability and application
processor-based end products. Unify SDK simplifies development and ongoing
software maintenance for IoT gateways and application processor-based end
devices.

Further details about the Unify SDK can be found in the [UnifySDK page](UnifySDK.md)

## Getting Started (WIP)

This section describes how to quickly get up and running with the different components of the Unify Host SDK.

Installing the unify packages on a raspberry:

- Download Unify binary release packages form the unify [Github](https://github.com/SiliconLabs/UnifySDK/releases)
- Raspbian 10 from the raspberry PI 4 release archive https://downloads.raspberrypi.org/raspios_lite_armhf/images/raspios_lite_armhf-2021-03-25
- flash sd card
- enable ssh
- install debian packages with apt
- Configure serial ports
- install mosquitto

### CPC
...
### Bluetooth HCI
...
### Open Thread Boarder Router
...

### ZigbeeD

....

### Z-Wave Protocol Controller
...
### Zigbee protocol controller
...

### Bluetooth AoX 
...
