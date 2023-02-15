#!/bin/bash

# manually compile and install mosquitto v2.0.11.

ARCH=$1

armhf_cmake_params='-DCMAKE_C_COMPILER=/usr/bin/arm-linux-gnueabihf-gcc -DCMAKE_CXX_COMPILER=/usr/bin/arm-linux-gnueabihf-g++ -DPACK_DEBIAN_PACKAGE_ARCHITECTURE=armhf \
  -DCPACK_FILE_NAME_EXTRA=bullseye -DPKG_CONFIG_EXECUTABLE=usr/bin/arm-linux-gnueabihf-pkg-config -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=armhf -DCONFIGURE_HOST=arm-linux'

arm64_cmake_params='-DCMAKE_C_COMPILER=/usr/bin/aarch64-linux-gnu-gcc -DCMAKE_CXX_COMPILER=/usr/bin/aarch64-linux-gnu-g++ -DPACK_DEBIAN_PACKAGE_ARCHITECTURE=arm64 -DCPACK_FILE_NAME_EXTRA=bullseye -DPKG_CONFIG_EXECUTABLE=usr/bin/aarch64-linux-gnu-pkg-config -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=arm64 -DCONFIGURE_HOST=arm64-linux'

cd /tmp \
    && wget https://mosquitto.org/files/source/mosquitto-2.0.14.tar.gz \
    && tar -xf mosquitto-2.0.14.tar.gz \
    && mv mosquitto-2.0.14 mosquitto \
    && mkdir /tmp/mosquitto/build

cmake_build_params=""

if [ "armhf" == "${ARCH}" ]; then
    echo "Building mosquitto for armhf"
    cmake_build_params=${armhf_cmake_params}
elif [ "arm64" == "${ARCH}" ]; then
    echo "Building mosquitto for arm64"
    cmake_build_params=${arm64_cmake_params}
else
    echo "Building mosquitto for default amd64"
fi

cd /tmp/mosquitto/build && \
cmake ${cmake_build_params} -DWITH_APPS=OFF -DWITH_PLUGINS=OFF -DDOCUMENTATION=OFF .. && make && sudo make install

