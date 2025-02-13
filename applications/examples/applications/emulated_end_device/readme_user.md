# Emulated End Device User's Guide

## Description

This application emulates a combination of end device and PC having all possible UCL capabilities
using the Unify Library. This is primarily developed to demonstrate Unify capabilities without having to implement and deploy custom protocol controller and its end device. 

## Installation

For installing please refer to the general installation method in
[Getting started](../../../../doc/getting_started.md#how-to-install-packages).

## Running the EED

### Using Systemd Service

Alternatively, you can run the EED using the Systemd service that is
installed with the Debian installer.
For more information, see
[Unify Framework User guide](../../doc/unify_readme_user.md).

### Using the Command Line

The best way to run the EED is by executing `uic-eed`.
You can configure the MQTT server, database path, and much more through command line options.
For details about options, run `uic-eed --help` command:

Make sure that you do not run the EED both as a service and using the
command line.

```console
pi@unify:~ $ sudo systemctl status uic-eed
* uic-eed.service - Unify Emulated End Device
   Loaded: loaded (/lib/systemd/system/uic-eed.service; disabled; preset: enabled)
   Active: active (running) since Tue 2024-12-23 14:50:26 IST; 1 day 20h ago
 Main PID: 13235 (uic-eed)
    Tasks: 2 (limit: 4915)
   CGroup: /system.slice/uic-eed.service
           |─13235 /usr/bin/uic-eed
```

If the service is running, stop it using the following command before
you run the application manually:

```console
pi@unify:~ $ sudo systemctl stop uic-eed
```

## Configuring EED
EED can run with single device support in both service mode as well as standalone(command line) mode. Single device support is configured with a device type that needs to be emulated. 
EED can support/emulate multiple end devices when run in command line mode and the CLI menu enables user to configure and/or reconfigure devices.

### Systemd Service
When running as a service, user can reconfigure the device type of choice by using the following command:
```console
pi@unify:~ $ sudo dpkg-reconfigure uic-eed
```

Follow this up with the following command to restart with new configuration:
```console
pi@unify:~ $ sudo systemctl restart uic-eed
```

### Standalone (Command-line)
Command line mode enables CLI menu which support many configuration option. However if user wants a quick start with a predefined device type then user shall use the `--eed.devicetype` command line option.
```console
pi@unify:~ $ uic-eed --eed.device_type thermostat
```

#### CLI Configuration 
Use the `help` command to get the complete list:
```console
EED>help
==================================================
Unify Command line interface Help:
==================================================
add_cluster_support Adds a cluster to an endpoint under a UNID from the attribute store
add_endpoint Adds an endpoint under a UNID in the attribute store with all clusters or can take      cluster-name(s) in UpperCamelCase separated with semi-colon      (Eg: add_endpoint 1,0,"OnOff;Level;ColorControl") or device type(Eg: onofflight) for customized cluster selection
add_unid Adds a UNID in the attribute store
# --- SNIP ---
remove_cluster_support Removes a cluster from  an endpoint under a UNID from the attribute Store
remove_endpoint Removes an endpoint under a UNID from the attribute Store
remove_unid Removes a UNID from the attribute Store
```

`add_cluster_support` Adds a cluster support to an existing endpoint under a UNID. It take UNID, endpoint ID and cluster as arguments. **Format:**
```console
EED>add_cluster_support <UNID>,<EPID>,<ClusterName>
```
`add_endpoint` Adds a new endpoint under an existing UNID. It takes UNID and endpoint ID and optionally cluster list or device-type as arguments. When the cluster list and device-type are missing, endpoint is configured with all supported clusters. **Format:**
```console
EED>add_endpoint <UNID>,<EPID>
```
*OR*
```console
EED>add_endpoint <UNID>,<EPID>,"<Cluster1>;<Cluster2>;..."
```
*OR*
```console
EED>add_endpoint <UNID>,<EPID>,<DeviceType>
```
> Note: Refer [Supported Device Types](#supported-device-types) section for list of supported device type that can be configured.

`add_unid` Adds a new device emulation with given UNID. **Format:**
```console
EED>add_unid <UNID>
```
`Note: The UNID is given in the above configuration is prepended with "emd-" to form actual UNID string`

`remove_cluster_support` Removes a cluster support from an existing endpoint under a UNID. It take UNID, endpoint ID and cluster as arguments. **Format:**
```console
EED>remove_cluster_support <UNID>,<EPID>,<ClusterName>
```
`remove_endpoint` Removes a new endpoint under an existing UNID. It takes UNID and endpoint ID. **Format:**
```console**
EED>remove_endpoint <UNID>,<EPID>
```
`remove_unid` Removes devices emulated under the given UNID from unify system. **Format:**
```console
EED>remove_unid <UNID>
```

**`Note: Along with the above command, the Unify Attribute Store commands are also available, which can be used if you want to update values that are not updatable via MQTT.`**

#### Supported Device Types:
* dimmablelight
* onofflight
* doorlock
* onofflightswitch
* lightsensor
* occupancysensor
* onoffpluginunit
* dimmablepluginunit
* extendedcolorlight
* windowcovering
* thermostat
* flowsensor
* allclustersdevice 

> As the name suggests `allclustersdevice` can be used when all supported clusters are needed.