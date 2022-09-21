#!/bin/bash
set -eu

# This script must be executed from a build folder containing the armhf debian
# packages for Raspberry Pi.

RPI_URL=https://downloads.raspberrypi.org/raspios_oldstable_lite_armhf/images/raspios_oldstable_lite_armhf-2022-09-07/2022-09-06-raspios-buster-armhf-lite.img.xz
if [ "$#" -eq 1 ]
then
    echo "Overriding Raspberry Pi OS URL with: $1"
    RPI_URL=$1
fi

SCRIPT_DIR=$(dirname "${BASH_SOURCE[0]}")
echo "-------------------------------------------------------------------"
echo "Fetching Raspberry Pi image from $RPI_URL"
curl -OL $RPI_URL
echo "-------------------------------------------------------------------"
echo "Extracting Image"
xz -vd *raspios-*-armhf-lite.img.xz && mv *raspios-*-armhf-lite.img unify_zwave_raspberrypi.img
echo "-------------------------------------------------------------------"
echo "Preparing Debian Packages"
unzip ./deb-packages/*armhf.zip
mkdir -p zwave_packages && mv \
    unify_*armhf/libuic_*.deb \
    unify_*armhf/uic-dev-gui_*.deb \
    unify_*armhf/uic-gms_*.deb \
    unify_*armhf/uic-image-provider_*.deb \
    unify_*armhf/uic-nal_*.deb \
    unify_*armhf/uic-upti-cap_*.deb \
    unify_*armhf/uic-upti-writer_*.deb \
    unify_*armhf/uic-upvl_*.deb \
    unify_*armhf/uic-zpc_*.deb \
    zwave_packages/
echo "mosquitto > zwave_packages/additional_packages.txt"

echo "-------------------------------------------------------------------"
echo "Creating SDCard image"
$SCRIPT_DIR/make_sd_card.sh unify_zwave_raspberrypi.img zwave_packages/

echo "-------------------------------------------------------------------"
echo "Shrinking unify_zwave_raspberrypi.img using pyshrink"
sudo $SCRIPT_DIR/pyshrink/pyshrink.sh -p unify_zwave_raspberrypi.img

echo "-------------------------------------------------------------------"
echo "Creating zip unify_zwave_raspberrypi.zip from unify_zwave_raspberrypi.img"
xz -vz -T0 unify_zwave_raspberrypi.img

echo "-------------------------------------------------------------------"
echo "Done, SDCard image is stored as unify_zwave_raspberrypi.zip and unify_zwave_raspberrypi.img"
