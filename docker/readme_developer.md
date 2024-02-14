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

## Dependencies

| Package         | Needed By | Needed/Used for |
|:----------------|:----------| :----------|
|libavahi-client-dev|OTBR|Multicast DNS Service Discovery.|
|libboost-filesystem-dev|attribute mapper|portable facilities to query and manipulate paths, files, and directories.|
|libboost-log-dev|uic log|to make logging significantly easier for the application developer.|
|libboost-program-options-dev|uic config|to make obtaining program options easier for the application developer.|
|libboost-system-dev|cmake|-|
|libboost-thread-dev|cmake|-|
|libedit-dev|uic stdin|generic line editing, history, and tokenization functions.|
|libjsoncpp-dev|OTBR|JSON reader and writer|
|libmbedtls-dev|CPCD|crypto and SSL/TLS library|
|libmosquitto-dev|uic mqtt|mosquitto mqtt library|
|libreadline-dev|uic stdin|consistency of user interface across discrete programs that need to provide a command line interface.|
|libsqlite3-dev|uic datastore<br>NAL<br>GMS<br>UPVL|SQL database engine.|
|libssl-dev|cmake|-|
|libyaml-cpp-dev|uic config<br>uic rust|parsing and emitting data in YAML|
||||
|build-essential|build-system|contains a list of packages that are required to create a Debian package|
|clang|build-system|provides a language front-end and tooling infrastructure for languages in the C language|
|cm-super-minimal|build-system|docs generation with Doxygen|
|curl|Docker|downloading files|
|doxygen|build-system|docs generation with Doxygen|
|dpkg|Docker|package manager for Debian-based systems|
|ghostscript|build-system|docs generation with Doxygen|
|git|build-system|download gecko_sdk and cpc-deamon|
|git-lfs|build-system|download gecko_sdk and cpc-deamon|
|graphviz|build-system|For docs generation|
|libasound2|ZAP generation|-|
|libgbm1|ZAP generation|-|
|ninja-build|build-system|Build system|
|nlohmann-json3-dev|ZigPC<br>ZPC<br>testframework<br> uic dotdot mqtt<br>uic smartstart management|Read JSON from a file.|
|nodejs|Dev-GUI|JavaScript runtime built on Chrome`s|
|npm|Dev-GUI|package manager for the JavaScript runtime environment Node.js|
|openjdk-11-jre|build-system|For docs generation|
|pkg-config|Portable Runtime<br>NAL<br>GMS|insert the compiler options on the command line|
|python3-pip|Docker|Install pip3 packages|
|ruby|testframework<br>zwave_transport|-|
|ruby-dev|testframework<br>zwave_transport||
|sudo|Docker|run commands with sudo permissions|
|texlive-extra-utils|build-system|For docs generation|
|texlive-font-utils|build-system|For docs generation|
|texlive-fonts-recommended|build-system|For docs generation|
|texlive-latex-extra|build-system|For docs generation|
|texlive-latex-recommended|build-system|For docs generation|
|texlive-plain-generic|build-system|For docs generation|
|unzip|Docker|Unzip files|
|valgrind|Tests|instrumentation framework for building dynamic analysis tools|
|wget|Docker|Downloading files|
|xauth|ZAP generation|-|
|xvfb|ZAP generation|-|
|zip|Zwave_Transports|Zip files|
