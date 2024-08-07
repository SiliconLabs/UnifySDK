#!/bin/bash

# Updating cpcd uart_device_file configuration with silink connection instance.
sed -i 's/uart_device_file: \/dev\/ttyACM0/uart_device_file: \/var\/ttyMULTIPROTOCOL/' /usr/etc/cpcd.conf

# Updating uic cpcd serial port configuration with silink connection instance.
sed -i 's/cpcd:\n  serial:.*/cpcd:\n  serial: \/dev\/ttyMULTIPROTOCOL/' /etc/uic/uic.cfg

(ip route save match ${MULTIPROTOCOL_DEVICE_IP} >/dev/null 2>&1) && \
(socat tcp-connect:${MULTIPROTOCOL_DEVICE_IP}:${MULTIPROTOCOL_PORT} pty,raw,echo=0,link=/var/ttyMULTIPROTOCOL &) || \
ln -sf ${MULTIPROTOCOL_DEVICE_TTY} /var/ttyMULTIPROTOCOL && \
sleep 2 && \
/usr/bin/cpcd --bind ecdh --key /var/lib/uic/cpcd-binding-key --conf /usr/etc/cpcd.conf && \
(/usr/bin/cpcd --key /var/lib/uic/cpcd-binding-key --conf /usr/etc/cpcd.conf &) && \

if [ "$MULTIPROTOCOL_SELECTIONS" == "Zigbee" ]; then

sleep 3 && \
(socat -v pty,link=/dev/ttyZigbeeNCP pty,link=/tmp/ttyZigbeeNCP &) && \
sleep 3 && \
(/usr/bin/stdbuf -o0 /usr/bin/zigbeed &) && \
zigpc --log.level d --zigpc.serial /dev/ttyZigbeeNCP --zigpc.datastore_file my_test.db "$@"

fi
