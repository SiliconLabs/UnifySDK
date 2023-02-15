# OpenThread Border Router User Guide

The OpenThread Border Router (OTBR) is included in a binary Debian package
with the Unify Host SDK. The version included is compiled from the GSDK
with CPCd connectivity and multiprotocol capabilities enabled.
It communicates to the 802.15.4 RCP using SPINEL via CPCd.
It can run alongside zigbeed using the same RCP, provided both
networks use the same 802.15.4 channel.

## Overview
The uic-otbr package contains a collection of compiled executables and
systemd services which performs the role of a Thread border router when enabled.

There are four OTBR services included in the uic-otbr package:
+ otbr-agent    - service to instantiate an OpenThread networking agent
+ otbr-web      - service to provide a web UI for configuration and management
+ otbr-firewall - service to implement ingress filtering rules
+ otbr-nat44    - service to allow connections to IPV4 endpoints

These services can be independently controlled, or they can be started and stopped together with the top-level **uic-otbr** service.

## Prerequisites
The uic-otbr package depends on the uic-cpcd package of the same version.

## Additional Resources

To get started running OTBR with CPCd, and for RCP troubleshooting information, see the
[Multiprotocol Setup](../../doc/getting_started_multiprotocol_cpc.md) page.

For more information about OpenThread and the OpenThread Border Router project, see the
[OpenThread Homepage](https://openthread.io) and the [OpenThread](https://github.com/openthread/openthread)
and [OpenThread Border Router](https://github.com/openthread/ot-br-posix) GitHub pages.
