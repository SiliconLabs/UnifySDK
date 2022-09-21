# cpc-hci-bridge User Guide

This page explains how to use the BlueZ Linux Bluetooth stack with
the multiprotocol RCP using the `cpc-hci-bridge` utility. This utility
allows the BlueZ stack to connect to the Coprocessor
Communication Daemon (CPCd) and communicate with the Bluetooth
Controller on the RCP via the Host Controller Interface (HCI) protocol.

## Prerequisites

First make sure you've flashed the proper multiprotocol RCP image with
BLE HCI support to the EFR32 (rcp-uart-802154-blehci). For details on
how to do that, see
[AN1333](https://www.silabs.com/documents/public/application-notes/an1333-concurrent-protocols-with-802-15-4-rcp.pdf).

Next, make sure you've successfully started and connected CPCd to the RCP
as described on the [Multiprotocol Setup page](../../doc/getting_started_multiprotocol_cpc.md).

If you followed the
[Unify Host SDK installation instructions](../../doc/getting_started.md),
the BlueZ Bluetooth stack should already be installed on your Raspberry Pi.
If not, BlueZ can be installed using `sudo apt install bluez`, or can be downloaded and compiled from source from the [offical BlueZ repository](https://git.kernel.org/pub/scm/bluetooth/bluez.git/about/).

## Running cpc-hci-bridge

cpc-hci-bridge connects to CPCd using the standard instance name cpcd_0,
and creates a numbered virtual serial device, for example /dev/pts/2.
The actual number may vary. For convenience, cpc-hci-bridge also creates
a symlink to the device from `pts_hci` in the working directory.

To start cpc-hci-bridge as a system service:

```console
pi@raspberrypi:~ $ sudo systemctl start uic-cpc-hci-bridge
```

Or from the command line, simply start the application:

```console
pi@raspberrypi:~ $ cpc-hci-bridge
```

Next, the BlueZ stack must be attached to the newly created virtual serial device,
where `<device>` is the name of the virtual serial device:

```console
pi@raspberrypi:~ $ sudo hciattach <device> any
```

Now BlueZ is running and any of the standard BlueZ tools may be used.
For example, to start the BlueZ CLI utility:

```console
pi@raspberrypi:~ $ sudo bluetoothctl
```

Another utility that comes with the standard Bluetooth tools, called `btmon`,
can be used to view the traffic going through the Bluetooth device.

## References

- [AN1333 Silicon Labs Multiprotocol RCP](https://www.silabs.com/documents/public/application-notes/an1333-concurrent-protocols-with-802-15-4-rcp.pdf)
