# Dev GUI Developer's Guide

This document is the developer's guide for the Developer GUI.
## Building

The Dev GUI can be cross compiled using the Docker image described in
[readme.md](../../../docker/readme_developer.md). This guide assumes you have
already built the docker image.

From the root of the UIC source directory runt the following:

``` bash
docker run -it --rm  -v`pwd`:`pwd` -w `pwd` uic_armhf
mkdir build
cd build
cmake -GNinja -DCMAKE_TOOLCHAIN_FILE=../cmake/armhf_debian.cmake ..
ninja
ninja deb
```

After cross-compiling, the Dev GUI's binaries (dev-gui-api & dev-gui-server) are
located in `./applications/dev_ui/dev_gui/`. The Debian installer package is
located in `./uic-dev-gui_<version>_armhf.deb`.

## Installing the Dev GUI

Follow the guide in [README.md](../../../README.md) for installing the Dev GUI.
