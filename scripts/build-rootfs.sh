#! /usr/bin/env bash
# -*- mode: Bash; tab-width: 2; indent-tabs-mode: nil; coding: utf-8 -*-
# vim:shiftwidth=4:softtabstop=4:tabstop=4:
# SPDX-License-Identifier: LicenseRef-MSLA
# SPDX-FileCopyrightText: Silicon Laboratories Inc. https://www.silabs.com

set -e
set -x

cat<<EOF
Usage:

ARCH=arm64 ./scripts/build-rootfs.sh
EOF

project="unifysdk"
debian_suite="bookworm"

# Can be overloaded from env eg: ARCH=arm64"
target_debian_arch=${ARCH:="$(dpkg --print-architecture)"}

debian_mirror_url="http://deb.debian.org/debian"
sudo="sudo"
machine="${project}-${debian_suite}-${target_debian_arch}-rootfs"
rootfs_dir="/var/tmp/var/lib/machines/${machine}"
MAKE="/usr/bin/make"
CURDIR="$PWD"
chroot="systemd-nspawn"
packages="debootstrap \
  binfmt-support \
  debian-archive-keyring \
  qemu-user-static \
  systemd-container \
  time \
"
case $target_debian_arch in
    amd64)
        export CMAKE_SYSTEM_PROCESSOR="x86_64"
        export CARGO_TARGET_TRIPLE="${CMAKE_SYSTEM_PROCESSOR}-unknown-linux-gnu"
        packages="${packages} qemu-system-x86"
        qemu_system="qemu-system-${CMAKE_SYSTEM_PROCESSOR}"
        ;;

    arm64)
        qemu_arch="aarch64"
        qemu_system="qemu-system-${qemu_arch}"
        export CMAKE_SYSTEM_PROCESSOR="${qemu_arch}"
        export CARGO_TARGET_TRIPLE="${CMAKE_SYSTEM_PROCESSOR}-unknown-linux-gnu"
        packages="${packages} qemu-system-arm"
        ;;

    *)
        CMAKE_SYSTEM_PROCESSOR="$ARCH"
        CARGO_TARGET_TRIPLE="${CMAKE_SYSTEM_PROCESSOR}-unknown-linux-gnu"
        qemu_arch="${ARCH}"
        qemu_system="qemu-system-${qemu_arch}"
        echo "error: Not supported yet"

        exit 1
        ;;
esac

${sudo} apt-get update
${sudo} apt install -y ${packages}
if [ -e "/var/lib/binfmts/qemu-${qemu_arch}" ]; then
    ${sudo} update-binfmts --enable "qemu-${qemu_arch}"
fi
qemu_file="/usr/bin/${qemu_system}"

if [ ! -d "${rootfs_dir}" ] ; then
    ${sudo} mkdir -pv "${rootfs_dir}"
    time ${sudo} debootstrap \
         --arch="${target_debian_arch}" \
	       "${debian_suite}" "${rootfs_dir}" "${debian_mirror_url}"
    ${sudo} chmod -v u+rX "${rootfs_dir}"
fi

# TODO: https://github.com/rust-lang/cargo/issues/8719#issuecomment-1516492970
env_vars="$env_vars CARGO_REGISTRIES_CRATES_IO_PROTOCOL=sparse"

# TODO: https://github.com/rust-lang/cargo/issues/10583
env_vars="$env_vars CARGO_NET_GIT_FETCH_WITH_CLI=true"

cargo_dir="/tmp/$USER/${machine}/${HOME}/.cargo"
${sudo} mkdir -pv  "${cargo_dir}"

case ${chroot} in
    "systemd-nspawn")
        rootfs_shell="${sudo} systemd-nspawn \
 --directory="${rootfs_dir}" \
 --machine="${machine}" \
 --bind="${CURDIR}:${CURDIR}" \
 --bind="${cargo_dir}:/root/.cargo" \
"
        if [ -e "${qemu_file}" ] ; then
            rootfs_shell="$rootfs_shell --bind ${qemu_file}"
        fi
        ;;
    *)
        rootfs_shell="${sudo} chroot ${rootfs_dir}"
        l="dev dev/pts sys proc"
        for t in $l ; do
            $sudo mkdir -p "${rootfs_dir}/$t"
            $sudo mount --bind "/$t" "${rootfs_dir}/$t"
        done
    ;;
esac

${rootfs_shell} \
    apt-get install -y -- make sudo util-linux

${rootfs_shell}	\
    ${MAKE} \
    --directory="${CURDIR}" \
    --file="${CURDIR}/helper.mk" \
    USER="${USER}" \
    ${env_vars} \
    -- \
    help setup default \
    target_debian_arch="${target_debian_arch}" \
    CMAKE_SYSTEM_PROCESSOR="${CMAKE_SYSTEM_PROCESSOR}" \
    CARGO_TARGET_TRIPLE="${CARGO_TARGET_TRIPLE}" \
    # EoL

sudo du -hs "/var/tmp/var/lib/machines/${machine}"
