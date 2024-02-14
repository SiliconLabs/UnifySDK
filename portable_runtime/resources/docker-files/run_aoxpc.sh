#!/bin/bash

(ip route save match ${AOXPC_DEVICE_IP} >/dev/null 2>&1) && (socat tcp-connect:${AOXPC_DEVICE_IP}:${AOXPC_PORT} pty,raw,echo=0,link=/var/ttyAOXPC &) || ln -sf ${AOXPC_DEVICE_TTY} /var/ttyAOXPC && sleep 2 && aoxpc --aoxpc.ncp.serial /var/ttyAOXPC "$@"
