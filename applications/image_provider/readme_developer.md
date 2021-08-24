# Image Provider Developer's Guide

For more information on UIC IMAGE PROVIDER, see [User Readme](./readme_user.md).

## Building the Image Provider Application

Use the docker image described in the `docker` folder. For
additional information, see [Docker Instructions](../../docker/readme_developer.md).

### Build Instructions

#### Cross Compiling for Raspberry Pi Using Docker

> The Raspberry Pi build is supported by Raspberry Pi OS Buster.

From the root of the UIC source directory, run the following:

```bash
docker run -it --rm  -v`pwd`:`pwd` -w `pwd` uic_armhf
mkdir build
cd build
cmake -GNinja -DCMAKE_TOOLCHAIN_FILE=../cmake/armhf_debian.cmake ..
ninja               # Build binaries
ninja doxygen       # Build doxygen documentation (optional)
ninja deb           # Build Debian Installers (optional)
```

After running this, the `uic-image-provider` binary is located in
`./applications/image_provider/armv7-unknown-linux-gnueabihf/debug/uic-image-provider`. The Debian
installer path are directly in the build folder:
`./uic-image-provider_<version>_armhf.deb`.

#### Cargo for Local Development

Enter `applications/image_provider` in the docker image and run the following commands:

If you're not running your build on an ARM system (e.g., on a Pi), omit
the target-parameters and look for your build-artifacts under `target/debug`.

``` bash
cargo build --target armv7-unknown-linux-gnueabihf
cargo deb --target armv7-unknown-linux-gnueabihf
```

After building, the image-provider binary will be located in
`target/armv7-unknown-linux-gnueabihf/debug/uic-image-provider` and
`target/armv7-unknown-linux-gnueabihf/release/uic-image-provider`. The Debian install is
located in
`./target/armv7-unknown-linux-gnueabihf/debian/uic-image_provider_<version>_armhf.deb`
