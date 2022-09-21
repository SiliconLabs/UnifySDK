# AoXPC User Guide

This user guide will guide the reader on how to install, configure and run the
Angle of Arrival/Departure Protocol Controller (AoXPC).

Readers of this document should be familiar with the basics and terms of
Direction Finding. To learn more about the theory of Direction Finding, see
[UG103.18: Bluetooth Direction Finding Fundamentals](https://www.silabs.com/documents/public/user-guides/ug103-18-bluetooth-direction-finding-fundamentals.pdf).

To get started with Silicon Labs Direction Finding Solution, see
[QSG175: Silicon Labs Direction Finding Solution Quick-Start Guide](https://www.silabs.com/documents/public/quick-start-guides/qsg175-direction-finding-solution-quick-start-guide.pdf).
This document explains how to prepare the radio boards with the necessary
firmware images.

## Limitations

- AoX stands for both Bluetooth Low Energy 5.1 direction finding features, Angle
of Arrival (AoA) and Angle of Departure (AoD). As of now, AoXPC supports only
AoA but not AoD.

- The asset tags in the AoA setup are considered to be homogeneous, i.e., every
asset tag should operate with the same CTE type (Connection, Connectionless or
Silicon Labs proprietary).

- The Unify specification describes 2 reporting modes, IQReport and AngleReport.
As of now, AoXPC supports only AngleReport.

## Operation

### Running AoXPC From the Command Line

The AoXPC can be started either from command line, or as a system service.
Running AoXPC as a system service is supported if there is only one locator
radio connected to the system.

```bash
sudo systemctl start uic-aoxpc
```

If the system hosts several locators, an AoXPC can be started for each
locator from the command line.

```bash
aoxpc --aoxpc.ncp.serial /dev/ttyACM1 --aoxpc.datastore_file aoxpc_1.db
```

> NOTE: Make sure to select separate datastore files for each locator.
A datastore file can be used only by one aoxpc instance at once.

### NCP Connection

In the AoA locator radios, the Bluetooth stack runs in a so called Network
Co-Processor (NCP) mode. There are 2 ways to set up an NCP connection with the
locator, either via serial port or via TCP/IP. The connection parameters can be
specified with the following CLI options:

```bash
  --aoxpc.ncp.serial arg (=/dev/ttyACM0)
                                        Serial port where NCP module is
                                        connected
  --aoxpc.ncp.baud_rate arg (=115200)   Baud rate of the serial connection
  --aoxpc.ncp.tcp arg                   TCP address of the connection
```

If the `aoxpc.ncp.tcp` is given, it takes precedence over the serial port.

To learn more about NCP mode, see
[AN1259: Using the v3.x Silicon Labs Bluetooth Stack in Network Co-Processor Mode](https://www.silabs.com/documents/public/application-notes/an1259-bt-ncp-mode-sdk-v3x.pdf).

### Updating NCP firmware

The firmware version of the NCP radio can be acquired using the
`--aoxpc.ncp.version` option. If necessary, the firmware can be updated using
the `--aoxpc.ncp.version <gbl_file_path>` option. The `<gbl_file_path>` is a
Gecko BootLoader (GBL) image file. The creation of GBL image files is described
in chapter _2.3 Creating Upgrade Images for the Bluetooth NCP Application_ of
[AN1086: Using the Gecko Bootloader with the Silicon Labs Bluetooth Applications](https://www.silabs.com/documents/public/application-notes/an1086-gecko-bootloader-bluetooth.pdf).

## Configuration

There are 2 ways for controlling the configuration parameters of the AoXPC.

1. JSON configuration file
2. AoXLocator cluster attributes

### JSON Configuration File

This option provides access to the complete list of the available configuration
parameters. The path of the JSON config file can be set with the
`--aoxpc.config` option. The format and the available configuration parameters
are documented in the
[AN1296: Application Development with Silicon Labs’ RTL Library](https://www.silabs.com/documents/public/application-notes/an1296-application-development-with-rtl-library.pdf)
in chapters _3.3 AoA Locator Configuration_, and _4.1 Positioning Configuration_.

### AoXLocator Cluster Attributes

A subset of the configuration parameters are also available as attributes of the
AoXLocator cluster. These attributes can be written by a remote client with the
`WriteAttributes` command. Once written, these attributes are stored in the
persistent storage and get restored from the datastore file upon startup.

### Configuration Precedence

If the JSON configuration file is provided, it always take precedence over the
AoXLocator cluster attributes. If neither JSON configuration file is present,
nor the AoXLocator cluster attributes are written, then the default value of
the configuration parameters come to effect.

### Configure AoXPC for Position Estimation

In order to operate the Positioning IoT Service, the coordinate and the
orientation of the AoXPC has to be configured. This task might be trivial for
2 locators, but the complexity can grow quickly with the number of locators.

AoA Configurator is a graphical tool shipped with Simplicity Studio that
simplifies this process by providing an editable 3D representation of your
topology and locator devices. See
[UG514: Using the Bluetooth Direction Finding Tool Suite](https://www.silabs.com/documents/public/user-guides/ug514-using-bluetooth-direction-finding-tool-suite.pdf)
chapter _4.2 AoA Configurator_ for details.

When the topology is configured, the result can be exported as a JSON
configuration file as described in chapter _3.5 Import & Export: Migrating Configuration Files_.

### Configuration Example

After determining the topology of the locators, let's see the 2 different
configuration methods in action. These methods will give the exact same result.

#### Using JSON configuration file

Save the following file as `my_aox_config.json`.

```json
{
  "locators": [
    {
      "id": "ble-pd-111111111111",
      "coordinate": {
        "x": 0.0,
        "y": 0.0,
        "z": 2.6
      },
      "orientation": {
        "x": 180.0,
        "y": 0.0,
        "z": 0.0
      }
    },
    {
      "id": "ble-pd-222222222222",
      "coordinate": {
        "x": 0.0,
        "y": 4.0,
        "z": 2.6
      },
      "orientation": {
        "x": 180.0,
        "y": 0.0,
        "z": 0.0
      }
    }
  ]
}
```

Start the aoxpc instances.

```bash
aoxpc --aoxpc.config my_aox_config.json --aoxpc.ncp.serial /dev/ttyACM1 --aoxpc.datastore_file aoxpc_1.db
aoxpc --aoxpc.config my_aox_config.json --aoxpc.ncp.serial /dev/ttyACM2 --aoxpc.datastore_file aoxpc_2.db
```

#### Using AoXLocator cluster attributes

Start the aoxpc instances.

```bash
aoxpc --aoxpc.ncp.serial /dev/ttyACM1 --aoxpc.datastore_file aoxpc_1.db
aoxpc --aoxpc.ncp.serial /dev/ttyACM2 --aoxpc.datastore_file aoxpc_2.db
```

Write AoXLocator cluster attributes from a remote client.

```mqtt
ucl/by-unid/ble-pd-111111111111/ep0/AoXLocator/Commands/WriteAttributes - {
  "PositionAndOrientation": {
    "CoordinateX": 0.0,
    "CoordinateY": 0.0,
    "CoordinateZ": 2.6,
    "OrientationX": 180.0,
    "OrientationY": 0.0,
    "OrientationZ": 0.0
  },
  "PositionAndOrientationValid": true
}
```

```mqtt
ucl/by-unid/ble-pd-222222222222/ep0/AoXLocator/Commands/WriteAttributes - {
  "PositionAndOrientation": {
    "CoordinateX": 0.0,
    "CoordinateY": 4.0,
    "CoordinateZ": 2.6,
    "OrientationX": 180.0,
    "OrientationY": 0.0,
    "OrientationZ": 0.0
  },
  "PositionAndOrientationValid": true
}
```
