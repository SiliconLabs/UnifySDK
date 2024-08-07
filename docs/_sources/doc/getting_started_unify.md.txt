# Getting Started with Unify

## Introduction

Unify SDK is a software framework that simplifies the developer experience, removing difficult parts of network control and network management as it relates to gateway and hub development in IoT applications. It can also provide Matter bridge functionality to other protocols that do not natively run matter.

## Prerequisites

- An RPi4 preinstalled with [Raspberry Pi OS Linux Version 12, Bookworm 64-bit](https://downloads.raspberrypi.org/raspios_lite_arm64/images/raspios_lite_arm64-2024-03-15/)

- At least one of the following, supported wireless radios is connected to the RPi4 via USB:
  - [Z-Wave module](https://www.silabs.com/wireless/z-wave) flashed with Z-Wave - NCP Serial API Controller.
  - [Zigbee module](https://www.silabs.com/wireless/zigbee) EFR32MG2X/EFR32xG22 running NCP for Zigbee or RCP for Multiprotocol
  - [Bluetooth module](https://www.silabs.com/wireless/bluetooth) EFR32xG22 running NCP Firmware for AoXPC


The following are the ways to get started with Unify..

1) Use [Portable Runtime](portable_runtime/readme_user.md)
for quick demo with Unify IoT gateway running on Windows, MacOS or Linux desktop.

```{toctree}
---
maxdepth: 1
titlesonly:
hidden:
---
portable_runtime/readme_user.md
```

2) Use [Unify on Raspberry Pi](getting_started.md)
for complete hands-on of Unify IoT gateway.

```{toctree}
---
maxdepth: 1
titlesonly:
hidden:
---
getting_started.md
```
