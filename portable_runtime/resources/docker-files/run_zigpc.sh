#!/bin/bash

(ip route save match ${ZIGPC_DEVICE_IP} >/dev/null 2>&1) && (socat tcp-connect:${ZIGPC_DEVICE_IP}:${ZIGPC_PORT} pty,raw,echo=0,link=/var/ttyZIGPC &) || ln -sf ${ZIGPC_DEVICE_TTY} /var/ttyZIGPC && sleep 2 && zigpc --zigpc.serial /var/ttyZIGPC "$@"
