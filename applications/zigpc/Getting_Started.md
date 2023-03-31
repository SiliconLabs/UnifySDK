
# Table Of Contents

- [Table Of Contents](#table-of-contents)
- [Preparing Hardware](#preparing-hardware)
- [Preparing the OS and installing required tools](#preparing-the-os-and-installing-required-tools)
- [Getting the source code on the devices](#getting-the-source-code-on-the-devices)
- [Docker Setup](#docker-setup)
- [Raspberry Pi Setup](#raspberry-pi-setup)
- [Compiling code for the Raspberry Pi](#compiling-code-for-the-raspberry-pi)
- [Getting the NCP WSTK going :](#getting-the-ncp-wstk-going-)
- [Debugging with visual studio code, Cleaning the code](#debugging-with-visual-studio-code-cleaning-the-code)
## Preparing Hardware

For this guide you need :

- 2 WSTK with BRD4163A radios
- 1 Raspberry Pi (3B+ or 4)
- 1 Network switch
## Preparing the OS and installing required tools

Easy to install tools :

- Git
- Visual Studio Code
- Raspberry Pi Imager : https://www.raspberrypi.com/software/
- Simplicity Studio 5 : https://www.silabs.com/developers/simplicity-studio
- Docker : https://www.docker.com/
- GDB - might require specific steps for macos : see https://gist.github.com/gravitylow/fb595186ce6068537a6e9da6d8b5b96d to sign gdb
- MQTT Explorer : https://mqtt-explorer.com/
- Nice to have : FileZila VNC Viewer
## Getting the source code on the devices

Clone the unify git repo :

    `git clone https://github.com/SiliconLabs/UnifySDK.git`

Initialise the git submodules of the repo :
`git submodule update --init --recursive`

Get a copy of the GSDK

[github.com/SiliconLabs/gecko_sdk](https://github.com/SiliconLabs/gecko_sdk)

public repo : 

    `git clone https://github.com/SiliconLabs/gecko_sdk`

You need to add the GSDK location to your environment variable GSDK_LOCATION

See also : Specific build instruction /projects/UIC/repos/uic/browse/doc/build_instructions.internal.md

## Docker Setup
After having installed docker desktop

To build and cross compile stuff ( inside host shell )

From the uic repository /docker folder :
To create new container for specific architectures :
 `./build_docker.sh <architecture> <container_name>`
EX : For the Pi
    `./build_docker.sh arm64 uic_arm64`
Ex : To run unit test 
    `./build_docker.sh amd64 uic_amd64`

Once the docker script is done : From the uic root folder run :

```shell
docker run -it --rm \
  --env GSDK_LOCATION \
  -v`pwd`:`pwd` \
  -w `pwd` \
  -v ${GSDK_LOCATION}:${GSDK_LOCATION} \
  uic_arm64
```

## Raspberry Pi Setup
Using the Raspberry Pi Imager, select the latest 64 bit Rasbian OS.
In advanced options of the Imager (Click the gear icon after selecting the OS), enable ssh, set username and password, hostname, enable wifi.

Add mosquitto to the system:
```shell
$sudo apt update
$sudo apt install mosquitto mosquitto-clients`
```
Install the packages from latest release :

Get the latest packages from the develop branch  :

https://github.com/SiliconLabs/UnifySDK/releases

Install them using (change name as required ) :

```shell
sudo dpkg -i uic-dev-gui_*_arm64.deb uic-gms_*_arm64.deb uic-image-provider_*_arm64.deb uic-upvl_*_arm64.deb uic-zigpc_*_arm64.deb
```

See user guide for more info : /projects/UIC/repos/uic/browse/doc/readme_user.md

For future MQTT Explorer use, add those lines to /etc/mosquitto/mosquitto.conf

```shell
listener 1883
allow_anonymous true
```

## Compiling code for the Raspberry Pi
On the development computer :

Go back to uic folder and enter your docker container using the previous docker run command (See [Docker Setup](#docker-setup)).
Set up the cmake build folder using :
```shell
mkdir build
cd build
cmake -GNinja -DCMAKE_TOOLCHAIN_FILE=../cmake/arm64_debian.cmake -DBUILD_ZPC=OFF -DCMAKE_BUILD_TYPE=Debug ..
ninja               # Build binaries
ninja doxygen       # Build doxygen documentation (optional)
ninja deb           # Build Debian Installers (optional)
```
Notice that this is telling cmake to use the specific target architecture ( arm64 ) toolchain and to enable debug symbol for future debugging.

If you want to reduce build time, you can specifically build a specific component :

```shell
ninja zigpc #Builds zigpc application
```

## Getting the NCP WSTK going :
Since Simplicity Studio adds time to this process, we will try to skip its usage. Be aware that you can generate new firmware with specific cluster using Simplicity Studio or by modifying the slcp file and then generating using the slc tool.

Thus, we will use the slc command line tool to create the firmware for the ncp.

Related user guide : https://www.silabs.com/documents/public/user-guides/ug520-software-project-generation-configuration-with-slc-cli.pdf

Follow the user guide installation process in order to install the tool, then add the slc folder to your system environment path.



You can find Zigbee sample apps for the ncp in the Gecko SDK folder under gecko-sdk/protocol/zigbee/app/ncp/sample-app/ncp-uart-hw

The Boatloader is under gecko-sdk/platform/bootloader/sample-apps/bootloader-storage-spiflash



In order to get the .s37 executable for the boatloader and the zigbee application :

```shell
slc generate ncp-uart-hw.slcp --with <BOARD_IDENTIFIER> -np -d output/cd output
make -f ncp-uart-hw.Makefile
```
Example board identifier
Thunderboard sense 2  : brd4166a
>Note : To synchronise the device and the gateway address table. add under define in the \<NCP-NAME\>.slcp
```shell
  - name: "EMBER_ADDRESS_TABLE_SIZE"
    value: "32"
```
This value must match EMBER_AF_PLUGIN_ADDRESS_TABLE_SIZE in zigpc/components/zigpc_gateway/libs/zigbee_host/gen/config

Once you have both .s37 files, flash using simplicity commander / simplicity studio

###Troubleshooting NCP issues and versioning

There are some common issues faced when updating to a new version of ZigPC or
a new version of the GeckoSDK

#### Issue #1: NCP and Gateway mismatch

After upgrading, it is possible to see ZigPC fail to start with the error:

>  ERROR: ezspErrorHandler 0x30

Or an equivalent error.

Usually this occurs because the version of the GeckoSDK on the gateway and the 
version on the NCP do not match. It's important that version matches so that the
EZSP communication libraries are the same on both the host and the radio.

This issue can be resolved in 2 ways: downgrading the version of the GeckoSDK to
match with the NCP or updating the NCP firmware to match the gateway.

To change the version of the GeckoSDK used by the gateway, download the desired
version of the GeckoSDK and keep note of the directory it is stored in. By
default, the provided Docker container downloads the latest version to the
'externals' folder in the uic project. This can be overridden by setting the
"GSDK\_LOCATION" environment variable with the new GeckoSDK and re-compiling the
ZigPC gateway.

To update the NCP firmware, re-generate the firmware as described above with the
matching version of the GeckoSDK. If you have hardware access, re-flash the NCP
as usual. However, if only remote access is possible, ZigPC can be used to
update the NCP firmware as described in the User Guide (section 'Updating NCP
Firmware'

####Issue #2: addressTable mismatch

When running the ZigPC, its possible for the gateway to fail to start with an
error describing:

> ASSERT addressTableSize mismatch

or a similar error.

Usually, this occurs because the address table size was not properly set in the
NCP. Some information is shared between the gateway and the NCP, specifically
related to the network information. In this case, the address table is used to
resolve the global EUI64 with the short ID commonly used on zigbee networks. The
size of these tables should match on both the gateway and NCP to ensure that no
data is lost when handling too many devices.

There are two ways of solving this: reducing the address table on the gateway or
increasing the address table size on the NCP.

The recommended method is to increase the address table size on the NCP. This
can be done by adding the "EMBER\_ADDRESS\_TABLE\_SIZE" to the .slcp file, as 
described in the section above. Alternatively, add a definition to the NCP
makefile setting the above define. Compiling with -DEMBER\_ADDRESS\_TABLE\_SIZE=
32 will also work.

It is possible to solve this by reducing EMBER\_AF\_ADDRESS\_TABLE\_SIZE on the
gateway, however this is NOT recommended as it can reduce the number of devices
that ZigPC will support

## Debugging with visual studio code, Cleaning the code

From phase 6 you should now have the ability to generate custom zigpc executables.

Transfer them to the pi using FileZila or scp.

make sure that no uic-zigpc from previously installed package are running using (use killall if need be)
```shell
sudo systemctl status uic-zigpc
chmod +x ./zigpc
sudo ./zigpc
```
>Note : zigpc should not usually be runned as superuser. There is currently a bug in datastore that forces this issue.

You should now be able to access the devgui of the gateway under raspberrypi.local:3080

For more information about the devgui see : /projects/UIC/repos/uic/browse/applications/dev_ui/dev_gui/readme_user.md

Debugging using visual studio code :
Install native debug extension : https://marketplace.visualstudio.com/items?itemName=webfreak.debug

Create a code workspace containing both the Gecko SDK folder and the UIC folder

As the Launch.json use :

```json
{
    "configurations": [
    {
        "type": "gdb",
        "request": "attach",
        "name": "Extended remote raspberrypi",
        "executable": "PATH_TO_UIC/uic/build/applications/zigpc/zigpc",
        "target": "extended-remote raspberrypi.local:2000",
        "remote": false,
        "cwd": "${workspaceRoot}",
        "valuesFormatting": "parseText",
        "stopAtConnect": true
    }
    ]
}
```

On the Pi install gdbserver
```shell
sudo apt-get install gdbserver
#Run ZigPc on GDBserver
sudo gdbserver localhost:2000 ./zigpc
```
Cleaning the code :
To clang format in zigpc (Modify path as needed)
```shell
find ../applications/zigpc/components -iname "*.c" -o -iname "*.h" -o -iname "*.cpp"  | xargs clang-format-10 -i
```
