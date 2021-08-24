# ZPC Developer's Guide

This doucument is intended for Unify developers who are trying to cross compile the
ZPC for Raspberry Pi and install and run it on a Raspberry Pi.

## Build Instructions

To cross compile the ZPC for Raspberry Pi, use Docker
on your workstation. If you haven't already built the Unify docker image, see
[readme.md](../../docker/readme_developer.md).

### Cross Compiling for Raspberry Pi Using Docker

> The Raspberry Pi build is supported by Raspberry Pi OS Buster.

From the root of the Unify source directory, run the following.

``` bash
docker run -it --rm  -v`pwd`:`pwd` -w `pwd` uic_armhf
mkdir build
cd build
cmake -GNinja -DCMAKE_TOOLCHAIN_FILE=../cmake/armhf_debian.cmake ..
ninja               # Build binaries
ninja doxygen       # Build doxygen documentation (optional)
ninja deb           # Build Debian Installers (optional)
```

After running this, the `zpc` binary is located in `./applications/zpc/zpc`. The
Debian installer path are directly in the build folder
`./uic-zpc_<version>_armhf.deb` and `./libuic_<version>_armhf.deb`

### Advanced Feature - Run Raspberry Pi Unit Tests in Docker on the Host

You can run the Raspberry Pi unit tests (and ZPC binary) within Docker on the
Host machine by using QEMU to emulate the ARM architecture and binfmt in the
kernel to tell the Linux OS to run the ARM binaries within QEMU.

To enable running the unit tests inside Docker on the Host machine, run the
following command in the Host (not within the docker container).

``` bash
docker run --user 0 --privileged --rm -it uic_armhf update-binfmts --enable
```

> NB: This command needs to be run once after each restart of the Host machine.

After enabling this you can run the unit tests by issuing following command in
the `uic_armhf` docker image in the build directory:

```bash
ninja test
```

## Install Instructions

Installation is documented in the main [README.md](../../README.md) file.
