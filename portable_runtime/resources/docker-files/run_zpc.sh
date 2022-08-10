#!/bin/bash

(ip route save match ${ZPC_DEVICE} >/dev/null 2>&1) && (socat tcp-connect:${ZPC_DEVICE}:4901 pty,raw,echo=0,link=/var/ttyZPC &) || ln -sf ${ZPC_DEVICE} /var/ttyZPC && sleep 2 && zpc --zpc.serial /var/ttyZPC "$@"
