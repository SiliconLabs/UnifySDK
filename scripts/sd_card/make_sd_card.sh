#!/bin/bash
set -eu

image=$1
deb_dir=$2

if [ "$(uname -s)" != "Linux" ]
then
  echo "This script can run only on Linux environment"
  exit 0
fi

if [ "$#" -ne 2 ]
then
  echo "Usage: make_sd_card.sh <Rapsbian image> <Directory with Unify deb packages to be installed>"
  echo "Raspbian images can be downloaded from https://downloads.raspberrypi.org/raspios_full_armhf/images/"
  exit 0
fi

if [ -f "$image" ]; then
    echo "$image exists."
else
    echo "$image does not exist."
    exit 1
fi

if [ -d "$deb_dir" ]; then
    echo "$deb_dir exists."
else
    echo "$deb_dir does not exist."
    exit 1
fi

if [ ! -d mnt ]
then
  mkdir mnt
fi

# Resize disk image with 1G to make room for our stuff
dd if=/dev/zero bs=1G count=1 >> $image
# Grow Partition size
parted_disk_end=`parted $image print |grep .img | cut -d: -f 2`
sudo parted $image resizepart 2 $parted_disk_end

loop_device=$(losetup -f)
sector_size=$(fdisk -l ${image} | grep "Sector size" | awk '{print $(NF-1)}')
partition_offset_sectors=$(fdisk -l ${image} | grep -e "83\sLinux" | awk '{print $(NF-5)}')
let "partition_offset_bytes=${sector_size}*${partition_offset_sectors}"

sudo apt install qemu-user-static
echo "Setting up Loop device on the image"
echo "-------------------------------------------------------------------"
sudo losetup ${loop_device} -o ${partition_offset_bytes} $image
# Grow filesystem to match new image size
sudo e2fsck -y -f ${loop_device}
sudo resize2fs ${loop_device}
if [ $? -eq 1 ]
then
  echo "Loop device failed"
  exit 1
fi

echo "Mount the Loop device"
echo "-------------------------------------------------------------------"
sudo mount ${loop_device} mnt/
if [ $? -eq 1 ]
then
  echo "Mounting failed. Detaching the loop device"
  losetup -d ${loop_device}
  exit 1
fi
ls mnt/
echo "Copying debian packages to the image"
echo "-------------------------------------------------------------------"
if [ -d mnt/home/pi/deb ]
then
  echo "mnt/home/pi/deb/ exists. Removing it"
  sudo rm -rf mnt/home/pi/deb/
fi
sudo mkdir -p mnt/home/pi/deb/
sudo cp -r $deb_dir/* mnt/home/pi/deb/
echo "Copying resolve.conf to the image. So that apt-get update works"
echo "-------------------------------------------------------------------"
sudo cp /etc/resolv.conf mnt/etc/resolv.conf
echo "Copying qemu-arm-static to image"
echo "-------------------------------------------------------------------"
sudo cp /usr/bin/qemu-arm-static mnt/usr/bin/
echo "Running chroot with the image"
echo "-------------------------------------------------------------------"

exit_code=0
sudo chroot mnt/ /bin/bash -c "\
export DEBIAN_FRONTEND=noninteractive && \
apt-get update --allow-releaseinfo-change && \
apt install -y /home/pi/deb/*.deb && \
if [ -f \"/home/pi/deb/additional_packages.txt\" ]; then cat /home/pi/deb/additional_packages.txt | xargs apt install -y; fi && \
systemctl enable uic-zpc && \
raspi-config nonint do_ssh 0 && \
rm -r /home/pi/deb" || exit_code=$?

if [ $exit_code -ne 0 ]
then
  echo "-------------------------------------------------------------------"
  echo "Installing packages in image failed. Cleaning up"
  echo "-------------------------------------------------------------------"
fi
echo "Removing qemu-arm-static from image"
echo "-------------------------------------------------------------------"
sudo rm mnt/usr/bin/qemu-arm-static
echo "Unmounting the image"
echo "-------------------------------------------------------------------"
sudo umount mnt/
echo "Detaching the loop device"
echo "-------------------------------------------------------------------"
sudo losetup -d ${loop_device}
if [ $exit_code -eq 0 ]
then
  echo "Done! You can flash the $image on the SDCard"
  echo "-------------------------------------------------------------------"
fi

exit $exit_code