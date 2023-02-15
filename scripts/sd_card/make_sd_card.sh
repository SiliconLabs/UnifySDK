#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(realpath "$(dirname "${BASH_SOURCE[0]}")")

DEB_ZIP_PATH_DEFAULT="./deb-packages/unify_*.zip"
IMAGE_PATH_DEFAULT="./unify_zwave_raspberrypi.img.xz"
PACKAGE_LIST_DEFAULT="mosquitto;uic-dev-gui;uic-gms;uic-image-provider;uic-nal;uic-upti-cap;uic-upti-writer;uic-upvl;uic-zpc"
RPI_URL_DEFAULT="./raspios-bullseye-arm64-lite.img.xz"

function cleanup()
{
  EXIT_CODE=$(( $? ))

  set +euo pipefail

  echo ""
  echo "Cleaning up:"
  echo " - Removing qemu-arm-static from image"
  rm -rf "${QEMU_BIN_PATH}" || true

  echo " - Removing comments from all lines of ${LD_CONFIG} and ${LD_PRELOAD}"
  sed -i 's/^\#//' "${LD_CONFIG}" 2>/dev/null || true
  sed -i 's/^\#//' "${LD_PRELOAD}" 2>/dev/null || true

  echo " - Unmounting the image and all submounts eg. '/dev/', '/sys/', '/proc' etc."
  umount -A --recursive "${RPI_IMG_MOUNT}" 2>/dev/null || true

  echo " - Detaching the loop device"
  losetup -d "${LOOP_DEVICE}" 2>/dev/null || true

  if [ ${EXIT_CODE} -eq 0 ]; then
    echo " - Shrinking ${RPI_IMG_PATH} using pyshrink."
    "${SCRIPT_DIR}/pyshrink/pyshrink.sh" -a -Z "${RPI_IMG_PATH}"
    EXIT_CODE=$(( $? ))
    mv "${RPI_IMG_PATH}.xz" ${IMAGE_PATH}
  fi

  echo " - Removing temp folder"
  rm -rf "${TEMP_DIR}" 2>/dev/null || true

  if [ ! -z ${SUDO_USER} ]; then
    echo " - Changing ownership for ${IMAGE_PATH} to be owned by ${SUDO_USER}";
    chown ${SUDO_USER}:${SUDO_USER} ${IMAGE_PATH}
  fi

  echo "
  -------------------------------------------------------------------
    $(if [ ${EXIT_CODE} -eq 0 ]; then echo SUCCESS - image ready at ${IMAGE_PATH}; else echo FAILED to create image.; fi)
  -------------------------------------------------------------------"

  exit ${EXIT_CODE}
}

function usage() {
  echo "
  Generate RPi SD Card image with packages pre-installed.
  Default installed packages are for Z-Wave certification purposes.

  Usage: ${0} [options]
    options:
      -d [DEB_ZIP_PATH]: path to zipped Debian packages.
         (default=${DEB_ZIP_PATH_DEFAULT})

      -i [IMAGE_PATH]: path to final image.
         (default=${IMAGE_PATH_DEFAULT})

      -p [PACKAGE_LIST]: Semi-colon or space separated quoted string, with a list of packages to install.
         (default=\"${PACKAGE_LIST_DEFAULT}\")

      -u [RPI_URL]: either a local file path or an URL.
         (default=${RPI_URL_DEFAULT})
"
}

DEB_ZIP_PATH="${DEB_ZIP_PATH:=${DEB_ZIP_PATH_DEFAULT}}"
IMAGE_PATH="${IMAGE_PATH:=${IMAGE_PATH_DEFAULT}}"
PACKAGE_LIST="${PACKAGE_LIST:=${PACKAGE_LIST_DEFAULT}}"
RPI_URL="${RPI_URL:=${RPI_URL_DEFAULT}}"

while getopts "hd:i:p:u:" opt; do
  case ${opt} in
    d )
      DEB_ZIP_PATH="${OPTARG}"
      ;;
    i )
      IMAGE_PATH="${OPTARG}"
      ;;
    p )
      PACKAGE_LIST="${OPTARG}"
      ;;
    u )
      RPI_URL="${OPTARG}"
      ;;
    h )
      ;&
    \? )
      usage
      exit 1
      ;;
  esac
done

[[ "${UID}" -eq 0 || "${EUID}" -eq 0 ]] || { echo "You need to have root privileges to run this script!"; exit 1; }
[[ "$(uname -s)" = "Linux" ]] || { echo "This script can run only on Linux environment"; exit 1; }

# Catch all exits, so mount and loop device can be cleaned up
trap cleanup EXIT

readonly PACKAGES="${PACKAGE_LIST//[;,]/ }"

readonly TEMP_DIR="$(mktemp -d -t "RPI_IMG_XXXX")"
readonly RPI_IMG_PATH="${TEMP_DIR}/""$(basename "${RPI_URL}" .xz)"

echo "
Validating parameters:
  DEB_ZIP_PATH: ${DEB_ZIP_PATH}
  IMAGE_PATH:   ${IMAGE_PATH}
  PACKAGE_LIST: \"${PACKAGES}\"
  RPI_URL:      ${RPI_URL}
  TEMP_DIR:     ${TEMP_DIR}
  RPI_IMG_PATH: ${RPI_IMG_PATH}"

echo "-------------------------------------------------------------------"
if [[ -f "${RPI_URL}" ]]; then
  echo "Extracting image from ${RPI_URL} to ${RPI_IMG_PATH}."
  xz --decompress --stdout "${RPI_URL}" > "${RPI_IMG_PATH}"
else
  # If RPI_URL is not an existing local file, go download and extract it.
  echo "${RPI_URL} not found locally - treating as URL."
  echo "Trying to download and extract image from ${RPI_URL} to ${RPI_IMG_PATH}."
  curl --silent --location "${RPI_URL}" | xz --decompress --to-stdout > "${RPI_IMG_PATH}"
fi

echo "Resize disk image with 1G to make room for installing packages."
dd if=/dev/zero bs=2G count=1 >> "${RPI_IMG_PATH}"

echo "Grow rootfs partition size"
readonly PARTED_DISK_END=$(parted "${RPI_IMG_PATH}" print |grep .img | cut -d: -f 2)
parted "${RPI_IMG_PATH}" resizepart 2 "${PARTED_DISK_END}"

readonly SECTOR_SIZE=$(fdisk -l "${RPI_IMG_PATH}" | grep "Sector size" | awk '{print $(NF-1)}')
readonly PARTITION_OFFSET_SECTORS=$(fdisk -l "${RPI_IMG_PATH}" | grep -e "83\sLinux" | awk '{print $(NF-5)}')
readonly PARTITION_OFFSET_BYTES=$(( SECTOR_SIZE*PARTITION_OFFSET_SECTORS ))

echo "Installing qemu-user-static."
apt install qemu-user-static

echo "Setting up Loop device on the image"
readonly LOOP_DEVICE=$(losetup --find --nooverlap --partscan --show --offset "${PARTITION_OFFSET_BYTES}" "${RPI_IMG_PATH}")

echo "Grow filesystem to fill the new partition size"
e2fsck -y -f "${LOOP_DEVICE}"
resize2fs "${LOOP_DEVICE}"

echo "Mount the Loop device"
readonly RPI_IMG_MOUNT="${TEMP_DIR}/rootfs"
mkdir -p "${RPI_IMG_MOUNT}"
mount "${LOOP_DEVICE}" -o rw "${RPI_IMG_MOUNT}"
pushd "${RPI_IMG_MOUNT}"
mount --bind /dev dev/
mount --bind /sys sys/
mount --bind /proc proc/
mount --bind /dev/pts dev/pts
popd

readonly DEB_FOLDER="${RPI_IMG_MOUNT}/home/pi/deb"
echo "Unzipping Debian packages ${DEB_ZIP_PATH} to the image folder ${DEB_FOLDER}"
unzip -j -o -d "${DEB_FOLDER}" "${DEB_ZIP_PATH}"

echo "Copying resolve.conf to the image. So that apt-get update works"
cp /etc/resolv.conf "${RPI_IMG_MOUNT}/etc/resolv.conf"

echo "Copying qemu-arm-static to image"
cp /usr/bin/qemu-arm-static "${RPI_IMG_MOUNT}/usr/bin/"
cp /usr/bin/qemu-aarch64-static "${RPI_IMG_MOUNT}/usr/bin/"

readonly LD_CONFIG="${RPI_IMG_MOUNT}/etc/ld.so.conf"
readonly LD_PRELOAD="${RPI_IMG_MOUNT}/etc/ld.so.preload"
echo "Adding comment to all lines in ${LD_CONFIG} and ${LD_PRELOAD}."
sed -i 's/^\(.*$\)/\#\1/' "${LD_CONFIG}"  2>/dev/null || true
sed -i 's/^\(.*$\)/\#\1/' "${LD_PRELOAD}" 2>/dev/null || true

echo "chroot'ing into image, at ${RPI_IMG_MOUNT}, to install packages."
ls -alh ${RPI_IMG_MOUNT}/**
readonly LOCAL_DEB_FOLDER="/home/pi/deb"
chroot "${RPI_IMG_MOUNT}" /usr/bin/bash -c " \
export DEBIAN_FRONTEND=noninteractive && \
cd ${LOCAL_DEB_FOLDER} && dpkg-scanpackages . /dev/null | gzip -9c > ${LOCAL_DEB_FOLDER}/Packages.gz && \
echo \"deb [trusted=yes] file://${LOCAL_DEB_FOLDER} ./\" > /etc/apt/sources.list.d/unify.list && \
apt update --fix-missing --allow-releaseinfo-change && \
apt upgrade -y && \
apt install -f -y ${PACKAGES} && \
apt clean && \
rm -rf ${LOCAL_DEB_FOLDER} /etc/apt/sources.list.d/unify.list && \
raspi-config nonint do_ssh 0 && \
echo \"Unify packages installed!\""

# Script done, leave it to cleanup() to eh... clean up. :P
