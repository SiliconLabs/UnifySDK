# Unify Dockerfile

This folder contains docker files with recipes for creating a docker image for building the various Unify applications.

The *Dockerfile* is supporting building all target applications (protocol controllers, UPVL, Developer GUI, etc.).

## Install Docker

The following documentation explains how to install and set up Docker.

- [Windows and OSX](https://www.docker.com/products/docker-desktop)
- [Debian/Ubuntu Linux Linux](https://docs.docker.com/engine/install/ubuntu/)

## Build Dockerfile Image

To build the docker image enter the `docker` folder and run:

``` bash
./build_docker.sh arm64 uic_arm64
```

This will build the Unify toolchain docker images and name it *uic_arm64*.
