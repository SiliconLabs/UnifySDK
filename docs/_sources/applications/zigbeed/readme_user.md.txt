# zigbeed User Guide

The Zigbee protocol stack runs on the host in a daemon called zigbeed.  Zigbeed
communicates with an 802.15.4 stack on the RCP via the SPINEL protocol. Zigbeed
connects via sockets to CPCd, which transports the SPINEL messages over the
serial bus to the RCP. A Zigbee host application connects to Zigbeed via a
PTY (pseudo terminal) and communicates using the EZSP protocol. Two host
applications are included with the Unify Host SDK: ZigPC, and Z3Gateway.

To get started running zigbeed with CPCd, see the
[Multiprotocol Setup](../../doc/getting_started_multiprotocol_cpc.md) page.

## Unify Configuration

When building and installing Zigbeed using the Unify stack, Zigbeed will run
very similarly to the standard multiprotocol setup.

A special service called "uic-zigbeed-socat.service" is started and creates a
'virtual' usb port which any Silicon Labs EZSP-based application can used.
The default path is '/dev/ttyZigbeeNCP' and is intended as a drop-in replacement
for any USB/serial ports used.

NOTE: The Zigbeed application is only built for arm32 bit targets using Unify
