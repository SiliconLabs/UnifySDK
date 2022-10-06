# Building Dockerfile for Target Architecture

The dockerfile can be used to build images for various target architectures.
The default architecture is *armhf* as this is the officially supported architecture for Unify.

To build for other architectures, set the first parameter of *build_docker.sh* to the desired arch, e.g.,

``` bash
./build_docker.sh amd64 uic_amd64
```

This will create a docker image with the tag *uic_amd64*.

Following ARCHs are tested and verified by Jenkins:

* armhf - For supporting Raspberry Pi
* amd64 - 64 bit x86_64
