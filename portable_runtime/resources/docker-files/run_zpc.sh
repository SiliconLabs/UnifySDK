#!/bin/bash

(ip route save match ${ZPC_DEVICE_IP} >/dev/null 2>&1) && (socat tcp-connect:${ZPC_DEVICE_IP}:4901 pty,raw,echo=0,link=/var/ttyZPC &) || ln -sf ${ZPC_DEVICE_TTY} /var/ttyZPC && sleep 2 && zpc --zpc.serial /var/ttyZPC "$@"
