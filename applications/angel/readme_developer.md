# ANGEL Developer's Guide

For more information about ANGEL, see [User Readme](./readme_user.md).

## Building the ANGEL Application

You can use the docker image described in the `docker` folder. For
additional information, see [Docker Instructions](../../docker/readme_developer.md).

### Build Instructions

#### Cross Compiling for Raspberry Pi Using Docker

> The Raspberry Pi build is supported by Raspberry Pi OS Buster.

From the root of the UIC source directory, run the following:

``` bash
docker run -it --rm  -v`pwd`:`pwd` -w `pwd` uic_armhf
mkdir build
cd build
cmake -GNinja -DCMAKE_TOOLCHAIN_FILE=../cmake/armhf_debian.cmake ..
ninja               # Build binaries
ninja doxygen       # Build doxygen documentation (optional)
ninja deb           # Build Debian Installers (optional)
```

After running this, the `uic-angel` binary is located in
`./applications/angel/armv7-unknown-linux-gnueabihf/debug/uic-angel`. The Debian
installer path are directly in the build folder
`./uic-angel-<version>_armhf.deb`.

#### Cargo for Local Development

Enter `applications/angel` in the docker image and run the following commands:

If you're not running your build on an ARM system (e.g., on a Pi), you can omit
the target-parameters and look for your build-artifacts under `target/debug`.

``` bash
cargo build --target armv7-unknown-linux-gnueabihf
cargo deb --target armv7-unknown-linux-gnueabihf
```

After building the upvl binary will be located in
`target/armv7-unknown-linux-gnueabihf/debug/uic-angel` and
`target/armv7-unknown-linux-gnueabihf/release/uic-angel`. The Debian install is
located in
`./target/armv7-unknown-linux-gnueabihf/debian/uic-angel_<version>_armhf.deb`
