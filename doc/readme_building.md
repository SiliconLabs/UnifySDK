# Unify Build Guide

This document is intended for systems integrators who are trying to cross compile the
Unify components. This guide explains how to build for a Raspberry Pi. 

For a list a of build dependencies please refer to the [Dockerfile](../docker/Dockerfile)

## Build Instructions

The Unify SDK uses a docker container to provide an appropriate build environment.
As a prerequisite docker must be installed on the build host. See 
[readme.md](../docker/readme_developer.md) for information about how to install
docker and build the unify sdk environment.

Some Unify components require the Silabs Gecko SDK installed.

The Gecko SDK can be downloaded from GitHub
[https://github.com/SiliconLabs/gecko_sdk/releases], select the file
gecko-sdk.zip . Currently, v4.0.1 is the only supported version. Set an
environment variable GSDK_LOCATION for this path.

### Getting the source

There are two ways to obtain the Unify source code, the simplest way is
to download the file uic-\<version\>.tar.gz from the [GitHub release
assets](https://github.com/SiliconLabs/UnifySDK/releases).

Alternatively the source can be downloaded using git directly. Note that
git clients needs to have _git-lfs_ support and the git client needs to
have authentication with github.

```
git@github.com:SiliconLabs/UnifySDK.git
```

### Cross Compiling for Raspberry Pi Using Docker

> The Raspberry Pi build is supported by Raspberry Pi OS Buster.

To start start the docker with the unify build information run the
following command from the root of the Unify source directory
```bash 
docker run -it --rm \
  --env GSDK_LOCATION \
  -v`pwd`:`pwd` \
  -w `pwd` \
  -v ${GSDK_LOCATION}:${GSDK_LOCATION} \
  uic_armhf
```

Once the docker is started a new prompt will be shown. Building
is done as follows:
``` bash
mkdir build
cd build
cmake -GNinja -DCMAKE_TOOLCHAIN_FILE=../cmake/armhf_debian.cmake ..
ninja               # Build binaries
ninja doxygen       # Build doxygen documentation (optional)
ninja deb           # Build Debian Installers (optional)
```

After running this, the `zpc` binary is located in `./applications/zpc/zpc`. The
Debian installer path are directly in the build folder
`./uic-<component>_<version>_armhf.deb` and `./libuic_<version>_armhf.deb`

### Advanced Feature - Run Raspberry Pi Unit Tests in Docker on the Host

The Unify unit tests can be run the Raspberry (and ZPC binary) within Docker on the
Host machine by using QEMU to emulate the ARM architecture and binfmt in the
kernel to tell the Linux OS to run the ARM binaries within QEMU.

To enable running the unit tests inside Docker on the Host machine, run the
following command in the Host (not within the docker container).

``` bash
docker run --user 0 --privileged --rm -it uic_armhf update-binfmts --enable
```

> NB: This command needs to be run once after each restart of the Host machine.

After enabling this unit tests can be run by issuing following command in
the `uic_armhf` docker image in the build directory:

```bash
ninja test
ninja cargo_test
```

## Porting the Unify SDK

Porting the Unify SDK to new platform using Debian Linux should be straight
forward. However, if components are ported to other Linux distributions it may
be necessary to update the startup scripts of the components. The Debian
packages provided with the Unify SDK all uses the system service `systemd` which
takes care of startup and shutdown of the components. For example, the file
[uic-zpc.service](applications/zpc/scripts/systemd/uic-zpc.service) is as
systemd config script. When porting to a new Linux system the systemd config
files should be used for reference. It is very important that all the Unify
components runs as a dedicated system user to prevent attackers from
compromising the entire system. Also note that most components need write access
to parts of the filesystem. The default path for writing files is
`/var/lib/uic`.
