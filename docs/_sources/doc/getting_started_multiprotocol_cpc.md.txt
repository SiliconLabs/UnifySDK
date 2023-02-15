# Multiprotocol Setup

## Prerequisites

Follow the directions in the
[Unify Host SDK getting started guide](getting_started.md)
to set up your Raspberry Pi 4 and install the Unify Host SDK Debian packages.

By default, the `uic-zigpc` service is configured to operate with the
single-protocol Zigbee NCP. This service runs the Unify Framework Zigbee Protocol
Controller. It can be configured to run with the multiprotocol RCP, as described
below. For now, stop the service using `systemctl`:

```console
pi@raspberrypi:~ $ sudo systemctl stop uic-zigpc
```

## RCP Setup

Use Studio with the GSDK to build the Multiprotocol RCP image for UART.
There are two versions, one that supports
OpenThread and Zigbee, and one that supports OpenThread, Zigbee, and BLE HCI.
If you plan to use BlueZ on the host, make sure to select the latter.
For more details on building the Multiprotocol RCP image see section 2.1 of
[AN1333](https://www.silabs.com/documents/public/application-notes/an1333-concurrent-protocols-with-802-15-4-rcp.pdf).

By default the RCP is configured to run at 115200 baud with hardware flow
control enabled. This will match the default configurations for CPCd. After
connecting the Raspberry Pi to the WSTK with a USB cable, verify that the
/dev/ttyACM0 device appears on the Pi.

## CPCd Setup

CPCd must be running before any of the protocol services can be started.
Start the `uic-cpcd` service using `systemctl`:

```console
pi@raspberrypi:~ $ sudo systemctl start uic-cpcd
```

Check to make sure it is running and successfully connected to the RCP:

```console
pi@raspberrypi:~ $ sudo systemctl status uic-cpcd
```

If the connection was successful you will see the following lines at the end
of the CPCd logs:

```bash
Feb 14 12:35:25 raspberrypi cpcd[8170]: [12:35:24:783] Info : Daemon startup was successful. Waiting for client connections
Feb 14 12:35:25 raspberrypi cpcd[8170]: [12:35:24:863] Info : Client is using library v2
Feb 14 12:35:25 raspberrypi cpcd[8170]: [12:35:24:878] Info : Opened connection socket for ep#1
Feb 14 12:35:25 raspberrypi cpcd[8170]: [12:35:24:878] Info : Endpoint socket #1: Client connected. 1 connections
```

If the connection failed, make sure the proper RCP image and bootloader are
flashed to the radio co-processor, and that the /dev/ttyACM0 exists on the Pi.
The WSTK serial vcom settings should be 115200 baud and rtscts handshake.
Also make sure you stopped the uic-zigpc service as described above, and
that no other service is attempting to access `/dev/ttyACM0`.

The first time connecting to an RCP, `uic-cpcd` uses ECDH to create a secure
binding between the host and the RCP. If CPCd failed to connect to the RCP,
and there are security error messages in the logs, there may be a problem with
the binding keys. If this is the case, the best solution is to
delete the binding keys on the host and the RCP and restart the `uic-cpcd`
service. To delete the host binding key:

```console
pi@raspberrypi:~ $ sudo rm /var/lib/uic/binding-key.key
```

(Note that the location of the host binding key differs in the Unify Host SDK
from the default location of /etc/binding-key.key used by the GitHub CPCd
package.)

To erase the binding key on the RCP, you must either erase and reflash the chip
for Series 1 parts, or flash the `cpc_secondary_uart_security_device_recovery`
sample application for Series 2 parts. Note that the `--unbind` argument for
cpcd will not work because unbind is not implemented by default on the RCP
for security reasons. For more information, see the [CPCd user guide](../applications/cpcd/readme_user.md).

## Zigbee

### Zigbeed and Socat

Start the zigbeed service using `systemctl`:

```console
pi@raspberrypi:~ $ sudo systemctl start uic-zigbeed
```

Verify that it is active:

```console
pi@raspberrypi:~ $ sudo systemctl status uic-zigbeed
```

If there are problems, make sure that CPCd is properly connected to the RCP.

Starting the `uic-zigbeed` service also starts the `uic-zigbeed-socat` service.
This service runs `socat`, which is a multipurpose relay tool for Linux. Socat
is used to set up a PTY (pseudo terminal) which will be used by the Zigbee
host application to connect to zigbeed. You can check the status of
`uic-zigbeed-socat` with `systemctl` as usual, and you can also check that
the default PTY /dev/ttyZigbeeNCP exists.

### Zigbee Host App

Zigbee host applications connect to zigbeed using the PTY created by socat
above, and communicate using the EZSP protocol. Since this interface is the
same as the single-protocol Zigbee NCP, any Zigbee host application built
for an NCP over UART can be used with zigbeed simply by starting it with
the correct PTY device name.

Two Zigbee host applications are shipped with the Unify Host SDK: Z3Gateway
and ZigPC.

Z3Gateway is the traditional Zigbee sample gateway application. Start it as
follows:

```console
pi@raspberrypi:~ $ sudo /usr/bin/Z3Gateway -p /dev/ttyZigbeeNCP
```

ZigPC is the Unify Framework's Zigbee Protocol Controller.  Start it like this:

```console
pi@raspberrypi:~ $ sudo /usr/bin/zigpc --log.level d --zigpc.serial /dev/ttyZigbeeNCP --zigpc.datastore_file my_test.db
```

In both cases, if the host application successfully connects to zigbeed,
a CLI prompt will appear.

## OpenThread Border Router

To run the OpenThread Border Router (OTBR), start the `uic-otbr` service:

```console
pi@raspberrypi:~ $ sudo systemctl start uic-otbr
```

Verify that the 'otbr-agent' service is running using `systemctl status`. Next, run the
`ot-ctl` CLI application.  This connects to OTBR and provides a CLI for
entering OpenThread commands:

Alternatively, verify that the 'otbr-web' service is running using `systemctl status`. Next, connect to the host
using a web browser to access the OTBR Web UI application. This provides a visual interface to configure a set of
network operations in conjunction with the `ot-ctl` CLI.

```console
pi@raspberrypi:~ $ sudo ot-ctl
```

## Bluetooth

To use the BlueZ Linux Bluetooth stack on the host, make sure you've flashed
the proper multiprotocol RCP image with BLE HCI support to the EFR32
(rcp-uart-802154-blehci), and that you've successfully connected CPCd to the
RCP as described above. Next, a small utility called `cpc-hci-bridge` must be
started to connect BlueZ to CPCd. See the [cpc-hci-bridge user guide](../applications/cpc-hci-bridge/readme_user.md)
