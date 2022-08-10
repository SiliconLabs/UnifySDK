# Portable runtime environment

## Unify Portable Runtime Environment

**Disclaimer**: Currently the Unify Portable Runtime environment only supports the `ZPC`.

**Prerequisites**:

1. [Z-Wave module](https://www.silabs.com/wireless/z-wave) flashed with Z-Wave - NCP Serial API Controller.
2. Docker version > 20.10.12 installed.
3. docker-compose version > 2.5.1
4. x86-64/amd64 CPU architecture.

The Unify Portable Runtime Environment strives to get Unify running as fast as
possible on your desktop. The portable runtime comes as a binary executable CLI,
together with Unify Debian packages, a docker-compose file and a tool which can
map a Silicon Labs devices into a Linux docker environment on Windows, MacOS and
Linux.

## Quick start

**WARNING**: If you start up the portable docker environment on a host already
running Unify Framework applications this will cause unexpected behavior.

Setting up the portable runtime environment should be as easy as downloading
portable_runtime_\<OS\>.zip file from the
[Unify's GitHub release page](https://github.com/SiliconLabs/UnifySDK/releases)
and unzipping it.

After unzipping make sure you only have a single [Z-Wave module](https://www.silabs.com/wireless/z-wave)
connected to your desktop and run the binary `portable_runtime_cli`.
This should setup the whole Unify Framework ecosystem.
An example of such can be seen below on a Windows machine.

```powershell
PS C:\Users\someuser\Downloads\portable_runtime_windows> .\unify_portable_cli.exe
Attempting to automap the connected device...
Mapping device with Silink...
Verifying Docker installation
Building Unify Portable Runtime...
Building docker image with unify_1.2.0_x86_64.zip
Unify is running!
Please visit http://localhost:3080 in your browser.
Change the 'Host' field to 'mqtt-broker' and press 'Connect'.

Press Ctrl-C to stop...
```

After starting the Portable Runtime, one can test it and control it with
the developer GUI on `http://localhost:3080`. Details on the developer GUI can
be found in the [Dev GUI User's Guide](../applications/dev_ui/dev_gui/readme_user.md).

**IMPORTANT**: To connect the DevGUI to the correct MQTT broker, change
`localhost` to `mqtt-broker`.

## Advanced options

To start up the environment, run the *unify_portable_cli* binary file.
With only one device connected, you can just run the *unify_portable_cli* application - for multiple connected devices, see below.
Once it is running, it will wait for you to press '*CTRL-C*' to end it.

```console
C:\portable_runtime_windows> unify_portable_cli.exe
```

### Z-Wave RF Region

The Z-Wave controller will default to running the `EU` region setting.
A commandline switch is available to choose a different region from the region list EU, US, US_LR, ANZ, HK, MA, IN, IS, RU, CN, JP, KR.
Here is an example on how to start it up in the US region.

```console
C:\portable_runtime_windows> unify_portable_cli.exe --zwave-rf-region US
```

### Multiple devices connected

Having multiple Thunderboard or WSTK boards connected via USB, you will have to specify which of the devices to use.
This can be done by providing the path to a serial device, a serial number or an IP address of the Z-Wave device which can run the *ZPC* Z-Wave controller.

Failing to specify a serial number, a list of all the connected devices will be printed.

```console
C:\portable_runtime_windows> unify_portable_cli.exe
Multiple devices detected and no device string was given.
Please provide the device serial number of the controller device eg. '-d 440262195'.
Here is a list of connected devices:
deviceCount=2
device(440262195) {
  serialNumber=440262195
  adapterType=JLink
  adapterLabel=J-Link Silicon Labs
  adapterId=2603A
  adapterName=BRD2603A Rev. A00
  adapterDescription=Unknown board
  ....
}
device(440247860) {
  serialNumber=440247860
  adapterType=JLink
  adapterLabel=J-Link Silicon Labs
  adapterId=4002A
  adapterName=BRD4002A Rev. A04
  adapterDescription=Wireless Starter Kit Mainboard
  ...
}
```

#### Serial no

Specifying the device's serial no. that you would like to use, can be done like so:

```console
C:\portable_runtime_windows> unify_portable_cli.exe --serial-no 440262195
```

Be aware that running it like this on Linux, requires libqt5network5 to be installed.
For Debian based distro, it can be installed like so:

```console
user@portable_runtime_environment:~$ sudo apt install -y --no-install-recommends libqt5network5
```

#### IP address

If you have eg. a WSTK board connected to the same network, you can specify it's IP like so:

```console
C:\portable_runtime_windows> unify_portable_cli.exe --ip-address 10.0.0.12
```

#### Direct device

This method is only supported on Linux.
By specifying the device path, you can map that device into the portable runtime.

```console
user@portable_runtime_environment:~$ unify_portable_cli --device /dev/ttyUSB0.
```

## Troubleshooting

In case you run into any issues, two log files are produced.
The two files, `docker.log` and `silink.log`, can be used during support.
