# Using the Co-Processor Communication Daemon (CPCd)

This documentation is a condensed format specifically for Unify.
The full CPCd documentation can be found on the
[CPCd GitHub page](https://github.com/SiliconLabs/cpc-daemon).

The key host software component at the heart of the multiprotocol architecture is
a multiplexer called CPCd (Co-Processor Communication daemon). CPCd runs on
the host and acts as a reliable and secure serial transport layer to the
RCP over either a UART or SPI serial bus. Each protocol stack running on the
host connects to CPCd in order to communicate with the RCP.

To get started using CPCd with Unify right away, see the [Multiprotocol Setup page](../../doc/getting_started_multiprotocol_cpc.md).

The rest of this page describes CPCd itself in detail.
This documentation is a condensed format specifically for Unify.
The full CPCd documentation can be found on the
[CPCd GitHub page](https://github.com/SiliconLabs/cpc-daemon).

CPCd enables one host system to communicate with a co-processor device
(also referred to as the secondary device) over a physical transport link
such as UART or SPI.

In CPC, data transfers between processors are segmented in sequential packets.
Transfers are guaranteed to be error-free and sent in order.

Multiple applications can send or receive on the same endpoint without worrying
about collisions.

A CPC daemon (CPCd) is provided to allow applications on Linux to interact
with a secondary running CPC.

The CPC daemon (CPCd) is distributed as three components:

* The daemon binary (cpcd)
* A library and associated header files that enable C applications to interact
  with the daemon (libcpc.so)
* A configuration file (cpcd.conf)

## Theory of Operation

CPCd uses Unix sockets configured as sequential packets to transfer data with
the Linux host applications. Data is then forwarded to the co-processor over a
serial link. The Unix sockets, used to transfer data with applications that use
the CPC Library (libcpc.so).

## CPCd Dependencies

CPCd requires MbedTLS to encrypt the endpoints. Minimal version required for Mbed-TLS is *2.7.0*.
`libmbedtls-dev` must be installed to compile from sources.
For development purposes, encryption can be disabled by providing `ENABLE_ENCRYPTION=FALSE`
`cmake ../ -DENABLE_ENCRYPTION=FALSE`

## Compiling CPCd and the CPC Library

CPCd is already compiled and distributed in a Debian package as part of the
Unify Host SDK installation.
See the [Multiprotocol Setup](../../doc/getting_started_multiprotocol_cpc.md) page to
get started using CPCd right away.

The build essential and CMake packages in Linux are required for this step.
Compile the CPC daemon in the source folder using the following commands:

```sh
mkdir build
cd build
cmake ../
make
```

## Installing CPCd

Super-user permissions may be required to install the daemon, cpclib, and the
configuration file.  These can be installed with the following commands:

```sh
make install
```

## Configuring CPCd

When running the daemon without arguments, it starts with the default
configuration file installed in the previous step (/usr/local/etc/cpcd.conf).
Note: in the Unify binary Debian package installation, the configuration
file is installed in /usr/etc/cpcd.conf.
To specify a different configuration file, use the --conf argument. For example:

```sh
cpcd --conf <configuration file path>
```

## Configuration file example

```sh
# Instance Name
# Optional, defaults to "cpcd_0"
# This string uniquely identifies the running cpcd instance. This value is
# passed in the initialization of the library
instance_name: cpcd_0

# Bus type selection
# Mandatory
# Allowed values : UART or SPI
bus_type: UART

# SPI device file
# Mandatory if spi chosen, ignored if uart chosen
#spi_device_file: /dev/spidev0.0

# SPI CS gpio
# Chip select gpio selection
#spi_cs_gpio: 8

# SPI RX IRQ gpio
# RX interrupt gpio selection
#spi_rx_irq_gpio: 22

# SPI bitrate.
# Optional if spi chosen, ignored if uart chosen. Defaults at 1000000
#spi_device_bitrate: 1000000

# SPI Mode.
# Optional if spi chosen, ignored if uart chosen. Defaults at SPI_MODE_0
#spi_device_mode: SPI_MODE_0

# UART device file
# Mandatory if uart chosen, ignored if spi chosen
uart_device_file: /dev/ttyACM0

# UART baud rate.
# Optional if uart chosen, ignored if spi chosen. Defaults at 115200
# Allowed values : standard UART baud rates listed in 'termios.h'
uart_device_baud: 115200

# UART flow control.
# Optional if uart chosen, ignored if spi chosen. Defaults at 'false'
# Allowed values are 'true' or 'false'
uart_hardflow: true

# BOOTLOADER Recovery Pins Enabled
# Set to true to enter bootloader via wake and reset pins
# If true, SPI_WAKE_GPIO and SPI_RESET_GPIO must be configured
#bootloader_recovery_pins_enabled: false

# BOOTLOADER WAKE gpio
# Wakeup gpio used by the bootloader
# Ensure BOOTLOADER_RECOVERY_PINS_ENABLED=true to use this pin
#bootloader_wake_gpio: 24

# BOOTLOADER RESET gpio
# Reset pin
# Ensure BOOTLOADER_RECOVERY_PINS_ENABLED=true to use this pin
#bootloader_reset_gpio: 23

# Prints tracing information to stdout
# Optional, defaults to 'false'
# Allowed values are 'true' or 'false'
stdout_trace: false

# Prints tracing information to a file located under TRACES_FOLDER
# Optional, defaults to 'false'
# Allowed values are 'true' or 'false'
trace_to_file: false

# Traces folder
# Optional, defaults to '/dev/shm/cpcd-traces'
# Folder mounted on a tmpfs is prefered
traces_folder: /dev/shm/cpcd-traces

# Enable frame trace
# Optional, defaults to 'false'
# Allowed values are 'true' or 'false'
enable_frame_trace: false

# Number of open file descriptors.
# Optional, defaults at 2000
# If the error 'Too many open files' occurs, this is the value to increase.
rlimit_nofile: 2000

# Disable the encryption over CPC endpoints
# Optional, defaults true
disable_encryption: true

# Binding key file
# Mandatory when security is used
# Must have 32 alphanumeric characters as the first line, representing a 128
# bit binding key
binding_key_file: /etc/binding-key.key
```

## Obtaining the Version of CPCd

If CPCd is started with the -v or --version argument, the daemon first prints
the version of CPCd and exits.

```sh
cpcd –v
```

## Tracing to the Standard Output (stdout)

When the configuration `stdout_trace` is enabled, the CPC daemon prints traces
to the console.

## Tracing to a File

When the configuration `trace_to_file` is enabled. the CPC daemon prints traces
to a file.  The tracing file name contains the date and timestamp. This file is
placed in the folder specified in the configuration `traces_folder`.

Note: Only enable tracing to a file when debugging, as log file size increases over time.

## Considerations

• The SPI driver uses a sysfs class GPIO as a chip select. Make sure the daemon
  has the proper permissions to access this GPIO.
• If the provided GPIO for the SPI chip select is already used by another
  driver, it needs to be deactivated and enabled as standard GPIO.
  In Linux this is usually done via the device tree.
• CPCd uses Unix sockets to exchange information with the Linux applications
  that use the CPC library.
  Only users with the appropriate permissions should be able to access these
  sockets. CPCd inherits the permission of the user who starts the CPC daemon.
• Make sure no other application is using the serial bus
  at the same time as CPCd.
• Sensitive information can be exposed when tracing to a file is enable.
  Only enable tracing during development, for debugging purposes only.
