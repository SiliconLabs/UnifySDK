# Portable runtime environment

## Unify Portable Runtime Environment

**Disclaimer**: Currently the Unify Portable Runtime environment only supports the Z-Wave Protocol Controller (`ZPC`).

**Prerequisites**:

1. [Z-Wave module](https://www.silabs.com/wireless/z-wave) flashed with Z-Wave - NCP Serial API Controller.
2. Docker version > 20.10.12 installed.
3. docker-compose version > 2.5.1
4. x86-64/amd64 CPU architecture.

The Unify Portable Runtime Environment strives to get Unify running as fast as
possible on your desktop. The portable runtime comes as a binary executable CLI,
together with Unify Debian packages, a docker-compose file and a tool which can
map a Silicon Labs device into a Linux docker environment on Windows, MacOS and
Linux.

## Quick start
**Note**: If you are running Linux, you need the right permissions to run the `Commander` and `Silink` tools.

For handling thes permissions, you either need to:

1. copy the Udev rules from `./resources/commander/99-jlink.rules` to the system folder `/etc/udev/rules.d/` and run `sudo udevadm control --reload && sudo udevadm trigger`.

   or

2. run all the commands as root with eg. `sudo ./unify_portable_cli....`.

**WARNING**: If you start up the portable docker environment on a host already
running Unify Framework applications, this will cause unexpected behavior.

Setting up the portable runtime environment should be as easy as downloading
`portable_runtime_<OS>.<zip/tar.bz2>` file from the
[Unify's GitHub release page](https://github.com/SiliconLabs/UnifySDK/releases)
and extracting it.

After extraction, make sure you only have a single [Z-Wave module](https://www.silabs.com/wireless/z-wave)
connected to your desktop and run the binary `portable_runtime_cli`.
This should set up the whole Unify Framework ecosystem.
An example of this can be seen below on a Windows machine.

```powershell
PS C:\Users\someuser\Downloads\portable_runtime_windows> .\unify_portable_cli.exe
Verifying Docker installation
devices: 1 device found.

Identified as 440285211 Serial API Controller
Mapping device with Silink...
Building Unify Portable Runtime...
Building docker image with unify_1.3.1_x86_64.zip
Starting Unify Portable Runtime...
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

If you do not specify a serial number, the portable runtime will attempt to
identify the controller device based on its application name. If that fails,
you will be asked to select the controller from a the list of the connected
devices.


```console
C:\portable_runtime_windows> unify_portable_cli.exe
Verifying Docker installation
devices: 2 devices found.
0: 440285211, J-Link EnergyMicro
0: 440265328, J-Link EnergyMicro

Attempting to identify the Serial API Controller...
Identification failed.
Serial number is not specified.

Device No. | Serial No. | Board No. | Freq. | Application name
-----------+------------+-----------+-------+------------------------------
         0 | 440285211  | 224300280 | EU    | unknown
         1 | 440265328  | 220500227 | EU    | unknown

Please select a device.
Note that the device's number may change when multiple boards are connected!
Type 0, 1, ... or x to cancel:
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

If you have eg. a WSTK board connected to the same network, you can specify its IP address like so:

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

In case you run into any issues, a log file is produced in the base directory, called `portable_runtime.log`.

## Additional subcommands

The portable runtime CLI includes additional convenience functions that allow
you to flash the application firmwares, query the DSK values, and list other
details of connected devices.

### Flash application

It is possible to flash the devices with the `flash-app` subcommand. To get help
on how to use this subcommand, use the `-h` switch:
```console
C:\portable_runtime_windows> unify_portable_cli.exe flash-app -h
```

The package contains several pre-built application firmwares. It is possible to
flash the pre-built applications using the `-n` flag and one of the following
names:  
`multilevel-sensor`, `power-strip`, `sensor-pir`, `serial-api`, `switch-on-off`,
and `wall-controller`.
```console
C:\portable_runtime_windows> unify_portable_cli.exe -s 440262211 flash-app -n switch-on-off
```

Or a relative path can be specified as an argument for an arbitrary application firmware using the `-p` flag.
```console
unify_portable_cli.exe -s 440262176 flash-app -u zwave_soc_switch_on_off-brd2603a-us.hex
```

Specifying the serial number is not required - in that case you will be asked to
select the device that you want to flash.

If the RF region is not defined by the `--zwave-rf` argument, then default value will be flashed (EU). It is possible to define the rf-region to be flashed to device from the region list: EU, US, US_LR, ANZ, HK, MA, IN, IS, RU, CN, JP, KR.
```console
unify_portable_cli.exe --zwave-rf US_LR -s 440262138 flash-app -n switch-on-off
```

After the application firmware is flashed to the device, the DSK of the End Device will be printed.
Note: In case of flashing serial-api, there is no DSK.

### Read Device Specific Key (DSK)

The `read-dsk` subcommand prints the DSK of a device. This DSK value can be used for secure inclusion.
The device to be queried can be selected from the list of devices interactively,
or by specifying the serial number `-s` flag before the subcommand.
```console
C:\portable_runtime_windows> unify_portable_cli.exe -s 440262138 read-dsk
```

### List connected devices

The `list-devices` subcommand allows to query the serial number, board number,
configured radio region, and application name of the connected devices, and can
be invoked as follows:
```console
C:\portable_runtime_windows> unify_portable_cli.exe list-devices
```
