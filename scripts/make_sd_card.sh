#!/bin/bash

image=$1
deb_dir=$2

if [ "$(uname -s)" != "Linux" ]
then
  echo "This script can run only on Linux environment"
  exit 0
fi

if [ "$#" -ne 2 ]
then
  echo "Usage: make_sd_card.sh <Rapsbian image> <Directory with UIC deb packages to be installed>"
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

if ls $deb_dir/libuic_*_armhf.deb &> /dev/null; then
  echo "libuic deb package found"
else
  echo "libuic deb package NOT found"
  exit 1
fi

if ls $deb_dir/uic-dev-cli_*_armhf.deb &> /dev/null; then
  echo "UIC Dev Cli deb package found"
else
  echo "UIC Dev Cli deb package NOT found"
  exit 1
fi

if ls $deb_dir/uic-dev-gui_*_armhf.deb &> /dev/null; then
  echo "UIC Dev Gui deb package found"
else
  echo "UIC Dev Gui deb package NOT found"
  exit 1
fi

if ls $deb_dir/uic-image-provider_*_armhf.deb &> /dev/null; then
  echo "UIC Image Provider deb package found"
else
  echo "UIC Image Provider deb package NOT found"
  exit 1
fi

if ls $deb_dir/uic-upvl_*_armhf.deb &> /dev/null; then
  echo "UIC UPVL deb package found"
else
  echo "UIC UPVL deb package NOT found"
  exit 1
fi

if ls $deb_dir/uic-zpc_*_armhf.deb &> /dev/null; then
  echo "UIC ZPC deb package found"
else
  echo "UIC ZPC libuic deb package NOT found"
  exit 1
fi

if [ ! -d mnt ]
then
  mkdir mnt
fi

loop_device=$(losetup -f)
sector_size=$(fdisk -l ${image} | grep "Sector size" | awk '{print $(NF-1)}')
partition_offset_sectors=$(fdisk -l ${image} | grep -e "83\sLinux" | awk '{print $(NF-5)}')
let "partition_offset_bytes=${sector_size}*${partition_offset_sectors}"

sudo apt install qemu-user-static
echo "Setting up Loop device on the image"
echo "-------------------------------------------------------------------"
sudo losetup ${loop_device} -o ${partition_offset_bytes} $image
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
sudo chroot mnt/ /bin/bash << "EOT"
apt-get update --allow-releaseinfo-change
apt install -y /home/pi/deb/*.deb
systemctl enable uic-zpc
raspi-config nonint do_ssh 0
EOT

echo "Removing qemu-arm-static from image"
echo "-------------------------------------------------------------------"
sudo rm mnt/usr/bin/qemu-arm-static
echo "Unmounting the image"
echo "-------------------------------------------------------------------"
sudo umount mnt/
echo "Detaching the loop device"
echo "-------------------------------------------------------------------"
sudo losetup -d ${loop_device}
echo "Done! You can flash the $image on the SDCard"
echo "-------------------------------------------------------------------"

