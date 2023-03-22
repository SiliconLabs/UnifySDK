#!/usr/bin/make -f
# -*- makefile -*-
# ex: set tabstop=4 noexpandtab:
# -*- coding: utf-8 -*

default: help zpc/default
	@echo "$@: TODO: Support more than $^ by default"
	@date -u

SELF?=${CURDIR}/helper.mk

project?=unifysdk

CMAKE_GENERATOR?=Ninja
export CMAKE_GENERATOR

build_dir?=build
sudo?=sudo

debian_codename?=bullseye

packages?=cmake ninja-build build-essential python3-full ruby clang
packages+=git-lfs unp time file
packages+=nlohmann-json3-dev
# TODO: remove for offline build
packages+=curl wget python3-pip
packages+=time

RUST_VERSION?=1.65.0
export PATH := ${HOME}/.cargo/bin:${PATH}

zpc_exe?=${build_dir}/applications/zpc/zpc
exes+=${zpc_exe}

zpc_cmake_options?=\
	-DBUILD_AOXPC=OFF \
	-DBUILD_CPCD=OFF \
	-DBUILD_DEV_GUI=OFF \
	-DBUILD_EMD=OFF \
	-DBUILD_EPC=OFF \
	-DBUILD_GMS=OFF \
	-DBUILD_IMAGE_PROVIDER=OFF \
	-DBUILD_NAL=OFF \
	-DBUILD_OTBR=OFF \
	-DBUILD_POSITIONING=OFF \
	-DBUILD_TESTING=ON \
	-DBUILD_UPTI_CAP=OFF \
	-DBUILD_UPTI_WRITER=OFF \
	-DBUILD_UPVL=OFF \
	-DBUILD_ZIGBEED=OFF \
	-DBUILD_ZIGPC=OFF

help: README.md
	@cat $<
	@echo ""
	@echo "# Available rules at your own risk:"
	@grep -o '^[^ ]*:' ${SELF} | grep -v '\$$' | grep -v '^#' | grep -v '^\.'
	@echo ""
	@echo "# Environment:"
	@echo "# PATH=${PATH}"
	@echo ""

setup/debian: ${CURDIR}/docker/target_dependencies.apt
	cat /etc/debian_version
	-${sudo} apt update
	${sudo} apt install -y $(shell cat $<)
	${sudo} apt install -y ${packages}
	@echo "$@: TODO: Support debian stable rustc=1.63 https://tracker.debian.org/pkg/rustc"

setup/rust:
	@echo "$@: TODO: Support https://tracker.debian.org/pkg/rustup"
	curl https://sh.rustup.rs -sSf | bash -s -- -y --default-toolchain ${RUST_VERSION}
	cat $${HOME}/.cargo/env
	@echo '$@: info: You might like to add ". $${HOME}/.cargo/env" to "$${HOME}/.bashrc"'
	-which rustc
	rustc --version
	cargo --version
	@echo "$@: TODO: Support stable version from https://releases.rs/ or older"

setup/python:
	@echo "$@: TODO: https://github.com/wbond/pybars3/issues/82"
	pip3 --version || echo "warning: Please install pip"
	pip3 install pybars3

cmake_url?=https://github.com/Kitware/CMake/releases/download/v3.21.6/cmake-3.21.6-Linux-x86_64.sh
cmake_filename?=$(shell basename -- "${cmake_url}")
cmake_sha256?=d460a33c42f248388a8f2249659ad2f5eab6854bebaf4f57c1df49ded404e593

setup/cmake:
	@echo "$@: TODO: remove for debian-12+"
	curl -O -L ${cmake_url}
	sha256sum  ${cmake_filename} \
		| grep "${cmake_sha256}"
	${SHELL} "${cmake_filename}" \
		--prefix=/usr/local \
		--skip-license
	rm -v "${cmake_filename}"
	cmake --version

setup-cmake: setup/cmake

setup/debian/bullseye: setup/debian setup/rust setup/python setup/cmake
	date -u

setup/debian/bookworm: setup/debian setup/rust setup/python
	date -u

setup: setup/debian/${debian_codename}
	date -u

git: .git/lfs
	git lfs version || echo "$@: warning: Please install git-lfs"
	git lfs status --porcelain || git lfs install
	time git lfs pull
	git lfs update || git lfs update --force
	git lfs status --porcelain

configure: ${build_dir}/CMakeCache.txt
	file -E $<

${build_dir}/CMakeCache.txt: CMakeLists.txt ${build_pre_list}
	cmake -B ${build_dir}

build: ${build_dir}/CMakeCache.txt
	cmake --build ${<D} \
		|| cat ${build_dir}/CMakeFiles/CMakeOutput.log
	cmake --build ${<D}
.PHONY: build

${build_dir}/%: build
	file -E "$<"

${exe}: build
	file -E $<

all: ${exes}
	file -E $<

test: ${build_dir}
	ctest --test-dir ${<}

distclean:
	rm -rf ${build_dir}

prepare: git

all/default: configure build test
	@date -u

zpc/configure: CMakeLists.txt
	cmake -B ${build_dir}  ${zpc_cmake_options}

zpc/build: zpc/configure build
	@date -u

zpc/test: ${build_dir}/applications/zpc/components/zwave_command_classes/test/
	ctest --test-dir ${<}

zpc/default: zpc/configure zpc/build
	@date -u

### @rootfs is faster than docker for env check

rootfs_dir?=/var/tmp/var/lib/machines/${project}

rootfs_shell?=${sudo} systemd-nspawn  \
		--machine="${project}" \
		--directory="${rootfs_dir}"
${rootfs_dir}:
	@mkdir -pv ${@D}
	time ${sudo} debootstrap --include="systemd,dbus" "${debian_codename}" "${rootfs_dir}"
	@${sudo} chmod -v u+rX "${rootfs_dir}"

clean/rootfs:
	-${sudo} mv -fv -- "${rootfs_dir}" "${rootfs_dir}._$(shell date -u +%s).bak"

rootfs/%: ${rootfs_dir}
	${sudo} file -E -- "${rootfs_dir}" \
		|| ${SELF} "${rootfs_dir}"
	${rootfs_shell} apt-get update
	${rootfs_shell} apt-get install -- make sudo
	${rootfs_shell}	\
	      	--bind="${CURDIR}:${CURDIR}" \
		${MAKE} \
			--directory="${CURDIR}" \
			--file="${CURDIR}/helper.mk" \
			HOME="${HOME}" \
			USER="${USER}" \
			-- "${@F}"

check/rootfs: prepare rootfs/check
	echo "# TODO only touched files"
	@echo "# ${project}: log: $@: done: $^"

test/rootfs: clean/rootfs rootfs/setup rootfs/distclean check/rootfs
	@echo "# ${project}: log: $@: done: $^"

### @Docker: is only for validation no need to rely on it

prepare/docker: Dockerfile prepare
	time docker build \
		--tag="${project}" \
		--file="$<" .
	@echo "# ${project}: log: $@: done: $^"

docker_workdir?=/usr/local/opt/${project}

docker/%: Dockerfile
	time docker run "${project}:latest" -C "${docker_workdir}" "${@F}"

test/docker: distclean prepare/docker docker/help docker/test
	@echo "# ${project}: log: $@: done: $^"
