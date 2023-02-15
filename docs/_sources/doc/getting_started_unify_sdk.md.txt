# Unify Framework Getting Started

## Setup base system

Unify Framework has a number of service applications which enables different basic
functionality. For unlocking all features of the Unify Framework, install the Debian
packages of the applications below.
It is recommended to use the Developer GUI for controlling the Unify Framework IoT Protocol Controllers such as `ZPC`,
`ZigPC` or `AoXPC`.

For installation steps refer to the [](how-to-install) section.

1. [Provisioning List (UPVL) User's Guide](../applications/upvl/readme_user.md).
2. [Group Manager Service (GMS) User's Guide](../applications/gms/readme_user.md).
3. [Name and Location service (NAL) User's Guide](../applications/nal/readme_user.md).
4. [OTA Image Provider User's Guide](../applications/image_provider/readme_user.md).
5. [Developer GUI User's Guide](../applications/dev_ui/dev_gui/readme_user.md).

## Choose an IoT protocol

For including an IoT protocol stack, you need to install and setup at least one protocol controller.

- [](zpc)
- [](zigpc)
- [](aoxpc)

(zpc)=

### Z-Wave Protocol Controller (ZPC)

**Prerequisite**: Required hardware for using the `ZPC` is a [Z-Wave module](https://www.silabs.com/wireless/z-wave)
which is flashed with a SerialAPI firmware.

The Z-Wave Protocol Controller allows Unify to control Z-Wave devices. Starting
quickly is achieved by just installing the `uic-zpc` Debian package. This should
automatically start up the `ZPC` after the configuration steps. You need to
provide the USB path for the Z-Wave module at the configuration steps.

A more in depth getting started guide specifically for the `ZPC` is
[ZPC User's Guide](../applications/zpc/readme_user.md).

(zigpc)=

### Zigbee Protocol Controller (ZigPC)

**Prerequisite**: Required hardware for using the `ZigPC` is a [Zigbee module](https://www.silabs.com/wireless/zigbee) which could be EFR32MG12/EFR32xG22
running NCP or CPC Firmware for Zigbee.

The `ZigPC` enables the Unify Framework to control Zigbee devices. Starting quickly
can be done by simply installing the `uic-zigpc` Debian package and configuring
the USB device at the configuration step. After installing, the protocol
controller should be started automatically as a systemd service.

For more information on getting started with the `ZigPC` is placed in
[ZigPC User's Guide](../applications/zigpc/readme_user.md).

(aoxpc)=

### Bluetooth Angle of Arrival/Departure Protocol Controller (AoXPC)

**Prerequisite**: Required hardware for using the `AoXPC` is a [Bluetooth module](https://www.silabs.com/wireless/bluetooth)
EFR32xG22 running NCP Firmware for AoXPC.

Lastly of the protocol controllers the above approach can again be used for the
`AoXPC`. Installing `uic-aoxpc`, going through configuration steps and it should
be running.

For more information on getting started with the `AoXPC` is found in
[AoXPC User Guide](../applications/aox/applications/aoxpc/readme_user.md).

### Evaluation after Installation

Once all Unify Framework Applications are installed and configured, one can evaluate the system via the Unify Framework Developer GUI.
Note that the RPi4 needs to be rebooted after installing the Debian packages for the first time.

After a reboot the Unify Framework Developer GUI can be accessed from a browser at [http://raspberrypi.local:3080](http://raspberrypi.local:3080).

_Note_ that the Unify Framework Developer GUI needs TCP access to the port 3080 and 1337
on the Raspberry Pi.

See the [Dev-GUI manual](../applications/dev_ui/dev_gui/readme_user.md) for more information about using this interface.
