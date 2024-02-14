# Z-Wave (ZPC)

## Introduction

Here we explain how to get up and running quickly to test out Unify on the Raspberry Pi reference platform for Z-Wave devices. 

## Prerequisites

- UZB7 Z-Wave dongle
- Unify installed on Raspberry Pi (as described [here](../../running_unify.md))

## Connect UZB7

- Connect the UZB7 to the USB port of the Raspberry Pi and type 
```console
pi@raspberrypi:~ $ dmesg|grep usb
```
- Make a note of the tty device
![dmesg](../../assets/img/zwave/dmesg.png)

## Configure ZPC 
- To configure ZPC type the following at a terminal
```console
pi@raspberrypi:~ $ sudo dpkg-reconfigure uic-zpc
```
- Enter the tty device from the previous step and select the correct region

## Run Unify
- To have Unify start at boot
```console
pi@raspberrypi:~ $ sudo systemctl enable uic-zpc
```
- To run Unify one time
```console
pi@raspberrypi:~ $ sudo systemctl start uic-zpc
```
## Connect to Unify UI

- Launch a web browser and connect to [http://raspberrypi.local:3080](http://raspberrypi.local:3080)
- In the top right of the UI enter "localhost" and port 1883, then click "Connect"
![devui](../../assets/img/zwave/devui.png)


