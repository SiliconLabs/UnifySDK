# Unify Framework Getting Started

## Setup base system

Unify Framework has a number of service applications which enables different basic
functionality. For unlocking all features of the Unify Framework, install the Debian
packages of the applications below.
It is recommended to use the Developer GUI for controlling the Unify devices in Unify Ecosystem including Emulated End Device(EED)

For installation steps refer to the [](how-to-install) section.

1. [Provisioning List (UPVL) User's Guide](../applications/upvl/readme_user.md).
2. [Group Manager Service (GMS) User's Guide](../applications/gms/readme_user.md).
3. [Name and Location service (NAL) User's Guide](../applications/nal/readme_user.md).
4. [OTA Image Provider User's Guide](../applications/image_provider/readme_user.md).
5. [Developer GUI User's Guide](../applications/dev_ui/dev_gui/readme_user.md).

## Choose an IoT protocol

For including an IoT protocol stack, you need to install and setup at least one protocol controller. 

For Zwave Protocol controller bring up, please refer to related chapter of [Z-Wave-Protocol-Controller documentation](https://siliconlabssoftware.github.io/z-wave-protocol-controller).

### Emulated End Device

For Unify bring up without any protocol controller , [EED](../applications/examples/applications/emulated_end_device/readme_user.md) can be used.

### Evaluation after Installation

Once all Unify Framework Applications are installed and configured, one can evaluate the system via the Unify Framework Developer GUI.
Note that the RPi4 needs to be rebooted after installing the Debian packages for the first time.

After a reboot the Unify Framework Developer GUI can be accessed from a browser at [http://raspberrypi.local:3080](http://raspberrypi.local:3080).

_Note_ that the Unify Framework Developer GUI needs TCP access to the port 3080 and 1337
on the Raspberry Pi.

See the [Dev-GUI manual](../applications/dev_ui/dev_gui/readme_user.md) for more information about using this interface.
