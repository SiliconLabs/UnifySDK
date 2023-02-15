# Getting Started with the Unify Host SDK

Before installing the Unify Host SDK components on our reference platform, a
Raspberry Pi 4 (RPi4), the following prerequisites must be fulfilled:

The Unify Host SDK targets our reference platform, a Raspberry Pi 4 (RPi4).
It is therefore recommended to set up a RPi4 for evaluating the full set of functionalities.
If you instead want a quick demo of a Z-Wave only Unify IoT gateway running on
your Windows, MacOS or Linux desktop, please try our
[Portable Runtime environment User guide](../portable_runtime/readme_user.md).

```{toctree}
---
maxdepth: 1
titlesonly:
hidden:
---
../portable_runtime/readme_user.md
```

The rest of this guide focuses on how to set up all of the applications and
services on a RPi4.

## Prerequisites

- An RPi4 preinstalled with [Raspberry Pi OS Linux Version 11, Bullseye 64-bit](https://downloads.raspberrypi.org/raspios_lite_arm64/images/raspios_lite_arm64-2022-09-26/)

- At least one of the following, supported wireless radios is connected to the RPi4 via USB:
  - [Z-Wave module](https://www.silabs.com/wireless/z-wave) flashed with Z-Wave - NCP Serial API Controller.
  - [Zigbee module](https://www.silabs.com/wireless/zigbee) EFR32MG12/EFR32xG22 running NCP for Zigbee or RCP for Multiprotocol
  - [Bluetooth module](https://www.silabs.com/wireless/bluetooth) EFR32xG22 running NCP Firmware for AoXPC

For details on setting up a bare RPi4, follow the setup instructions in the
[Official Raspberry Pi User Guide](https://projects.raspberrypi.org/en/projects/raspberry-pi-setting-up),
and ensure a supported radio module is attached to your RPi4.

For running Unify Framework it is required to have a running MQTT broker to which the Unify Framework can connect.
To get an MQTT broker installed on the RPi4:

```console
pi@raspberrypi:~ $ sudo apt update
pi@raspberrypi:~ $ sudo apt install mosquitto
```

For more information regarding logging and configuration of eg. the Mosquitto
broker, see the [Unify Framework User Guide](unify_readme_user.md).

```{toctree}
---
maxdepth: 1
titlesonly:
hidden:
---
unify_readme_user.md
```

(how-to-install)=

## How to install packages

The Unify Host SDK distribution targets the reference platform Debian Bullseye
platform. Binary Debian packages are provided with the SDK and they are
available in a Zip file eg. unify_x.y.z_arm64.zip on
[Unify's GitHub release page](https://github.com/SiliconLabs/UnifySDK/releases).

The simplest way to install the Unify components is to use the `apt` tool.
Transfer the `.deb` packages to the target via eg. SCP. Make sure to use either
a local relative path with `./` or absolute paths. Otherwise apt will look
for the package in the wrong place and fail.

Install a specific Unify package:

```console
pi@raspberrypi:~ $ sudo apt install uic-sdk-dir/uic-<component>_<version>_arm64.deb
```

Install all packages:

 ```console
pi@raspberrypi:~ $ sudo apt install uic-sdk-dir/*.deb
```

Update installed packages:

 ```console
pi@raspberrypi:~ $ sudo apt reinstall uic-sdk-dir/*.deb
```

### Verify the configuration

After installing the desired Unify Host SDK packages, make sure the configuration is set correctly.

```console
pi@raspberrypi:~ $ cat /etc/uic/uic.cfg
zpc:
  serial: /dev/ttyUSB0
  rf_region: EU
  datastore_file: /var/lib/uic/zpc.db
zigpc:
  serial: /dev/ttyACM0
  datastore_file: /var/lib/uic/zigpc.db
cpcd:
  serial: /dev/ttyACM0
```

If your configuration is wrong, you can either edit the file directly or reconfigure a specific package.
After making changes to the configuration make sure to either reboot or restart the service.

```console
pi@raspberrypi:~ $ sudo dpkg-reconfigure uic-zpc
```

### Verifying if an application is running

The Unify applications all install SystemD service files for handling the starting and stopping of the applications on a RPi4.
You can check if a Unify Host SDK application is running by using `systemctl`.
For information on how to handle SystemD services using `systemctl`, see [the systemctl man pages](https://man7.org/linux/man-pages/man1/systemctl.1.html).

Here we can see that the `uic-zpc` service is loaded and active:

```console
pi@raspberrypi:~ $ systemctl status uic-zpc.service
* uic-zpc.service - Unify Z-Wave protocol controller
   Loaded: loaded (/lib/systemd/system/uic-zpc.service; enabled; vendor preset: enabled)
   Active: active (running) since Tue 2022-08-02 14:14:58 CEST; 8s ago
 Main PID: 9471 (zpc)
    Tasks: 1 (limit: 4915)
   CGroup: /system.slice/uic-zpc.service
           -9471 /usr/bin/zpc

aug 02 14:15:02 raspberrypi zpc[9471]: 2022-Aug-02 14:15:02.158423 <W> [uic_stdin_process] stdin is not a tty, input will not be handled.
aug 02 14:15:02 raspberrypi zpc[9471]: 2022-Aug-02 14:15:02.166045 <i> [mqtt_client] Connection to MQTT broker localhost:1883 established.
aug 02 14:15:02 raspberrypi zpc[9471]: 2022-Aug-02 14:15:02.168337 <i> [mqtt_client_fsm_connecting] We are connected to the MQTT broker.
```

## Unify Framework

If you are interested in building an IoT Gateway based on the Unify Framework refer to
the [Unify Framework Getting Started](getting_started_unify_sdk.md).

```{toctree}
---
maxdepth: 1
titlesonly:
hidden:
---
Getting Started with the Unify Framework <getting_started_unify_sdk.md>
```

## Multiprotocol Setup

If you are interested in setting up multiprotocol CPC-based applications
see the [Multiprotocol Setup page](getting_started_multiprotocol_cpc.md).

```{toctree}
---
maxdepth: 1
titlesonly:
hidden:
---
Getting Started with the Multiprotocol Setup <getting_started_multiprotocol_cpc.md>
```
