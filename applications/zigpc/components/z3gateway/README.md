# Z3Gateway API Readme
This component was derived from the z3gateway generated from Simplicity Studio
(v4 REL) App Builder and with the Silicon Laboratories Inc. Gecko SDK v.2.7.

The modified Makefile is used to generate the Z3Gateway component as a library.
In addition to the source files under `libs/`, the files under `src/` and
`include/` are also packaged to create the library.

## File structure

    |- Makefile             App Builder generated Makefile with modifications
                            needed to build as a CMake project
    |- src/                 Interfacing functionality between App Builder and
                            ZigPC components
    |- include/             public API for interfacing functionality between
                            App Builder and ZigPC components
    |- libs/
        |- appbuilder/      Files generated from App Builder(except stack files)
            |- stack        Modified files from ZNet stack from
                            `<super>/protocol/zigbee/`

NOTE: The currently supported compiler is GCC.

## Modifications to Z3Gateway Project

### Makefile changes

The appbuilder generated Makefile is modified as follows:
1. Options to pass in the following parameters:
    - COMPILER
    - LINKER
    - ARCHIVE
    - STD
2. Create variable `GSDK_LOCATION` to be passed in as a parameter as well as
   replace GSDK root paths with this variable
3. Define the API headers using `GSDK_LOCATION` instead of in
   `libs/appbuilder/z3gw.h`
    - EMBER_AF_API_DEVICE_TABLE
    - EMBER_AF_API_EZSP_PROTOCOL
    - EMBER_AF_API_EZSP
    - EMBER_AF_API_GREEN_POWER_COMMON
    - EMBER_AF_API_LINKED_LIST
    - EMBER_AF_API_NETWORK_CREATOR
    - EMBER_AF_API_NETWORK_CREATOR_SECURITY
    - EMBER_AF_API_NETWORK_STEERING
    - EMBER_AF_API_SCAN_DISPATCH
    - EMBER_AF_API_EZSP_SECURE
    - EMBER_AF_API_CRC
    - EMBER_AF_API_ENDIAN
    - EMBER_AF_API_HAL
    - EMBER_AF_API_RANDOM
    - EMBER_AF_API_SYSTEM_TIMER
    - EMBER_AF_API_UPDATE_TC_LINK_KEY
    - EMBER_AF_API_COMMAND_INTERPRETER2
4. Create macro `ENABLE_ZIGPC_CHANGES` to encompass changes made from the
   appbuilder generated code

## GSDK EmberAF Host File Changes

1. Split emberAfMain functionality into init and tick functions:
    - `GSDK_LOCATION`/protocol/zigbee/app/framework/util/af-main-host.c: Retain all
      calls until COMMAND_READER_INIT() to emberAfMain()
    - `GSDK_LOCATION`/protocol/zigbee/app/framework/util/af-main-host.c: Add all
      calls after COMMAND_READER_INIT() in emberAfMain() to a new function,
      emberAfMainTick()

### GSDK Linux Serial File Changes
1. Remove SIGINT signal handler in linux-serial.c in order for the ZigPC SIGINT
   handler to allow graceful shutdown of components
    - NOTE: on shutdown, emberSerialCleanup() will be called

### Remove CLI interface functionality
1. Undefine macro ZA_CLI_FULL
2. Remove init() and tick() function calls to Gateway Plugin
    - znet-bookkeeping.c: emberAfPluginGatewayInitCallback() in emberAfInit()
    - znet-bookkeeping.c: emberAfPluginGatewayTickCallback() in emberAfTick()
3. Prevent second CLI i/o process from spawning
    - `GSDK_LOCATION`/protocol/zigbee/app/util/serial/linux-serial.c: Remove call to
      serialInitInternal(port) in emberSerialInit()
    - `GSDK_LOCATION`/protocol/zigbee/app/util/serial/linux-serial.c: Remove the
      SIGINT handler in installSignalHandler()

### List of GSDK Zigbee Plugins Used in API exposed

The following plugins are used in the API exposed under `/include`:
1. Network Creator: For PAN creation
2. Network Creator Security: For adding end devices to the PAN
3. Device Table: For getting join callbacks + tracking NodeID changes in the PAN
