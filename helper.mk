#!/usr/bin/make -f
# -*- makefile -*-
# ex: set tabstop=4 noexpandtab:
# -*- coding: utf-8 -*

default: help all/default
	@echo "$@: TODO: Support more than $^ by default"
	@date -u

SELF?=${CURDIR}/helper.mk

project?=unifysdk

# Allow overloading from env if needed
# VERBOSE?=1
BUILD_DEV_GUI?=OFF
BUILD_IMAGE_PROVIDER?=ON

cmake_options?=-B ${build_dir}

CMAKE_GENERATOR?=Ninja
export CMAKE_GENERATOR

build_dir?=build
sudo?=sudo

debian_codename?=bookworm

packages?=cmake ninja-build build-essential python3-full ruby clang
packages+=git-lfs unp time file
packages+=nlohmann-json3-dev
# TODO: remove for offline build
packages+=curl wget python3-pip
packages+=time

# Extra for components, make it optional
packages+=python3-jinja2
packages+=yarnpkg

rust_url?=https://sh.rustup.rs
RUST_VERSION?=1.71.0
export PATH := ${HOME}/.cargo/bin:${PATH}


# Allow overloading from env if needed
ifdef VERBOSE
CMAKE_VERBOSE_MAKEFILE?=${VERBOSE}
cmake_options+=-DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
endif

ifdef BUILD_DEV_GUI
cmake_options+=-DBUILD_DEV_GUI=${BUILD_DEV_GUI}
ifeq (${BUILD_DEV_GUI}, ON)
packages+=nodejs
endif
endif

ifdef BUILD_IMAGE_PROVIDER
cmake_options+=-DBUILD_IMAGE_PROVIDER=${BUILD_IMAGE_PROVIDER}
endif

# Allow to bypass env detection, to support more build systems
ifdef CMAKE_SYSTEM_PROCESSOR
cmake_options+=-DCMAKE_SYSTEM_PROCESSOR="${CMAKE_SYSTEM_PROCESSOR}"
export CMAKE_SYSTEM_PROCESSOR
else
# CMAKE_SYSTEM_PROCESSOR?=$(shell uname -m)
endif

ifdef CARGO_TARGET_TRIPLE
cmake_options+=-DCARGO_TARGET_TRIPLE="${CARGO_TARGET_TRIPLE}"
export CMAKE_TARGET_TRIPLE
endif


help: README.md
	@cat $<
	@echo ""
	@echo "# Available rules at your own risk:"
	@grep -o '^[^ ]*:' ${SELF} | grep -v '\$$' | grep -v '^#' | grep -v '^\.'
	@echo ""
	@echo "# Environment:"
	@echo "# PATH=${PATH}"
	@echo ""

setup/debian: ${CURDIR}/docker/target_dependencies.apt ${CURDIR}/docker/host_dependencies.apt
	cat /etc/debian_version
	-${sudo} apt update
	${sudo} apt install -y $(shell sort $^ | sed -e 's|//.*||g' )
	${sudo} apt install -y ${packages}
	@echo "$@: TODO: Support debian stable rustc=1.63 https://tracker.debian.org/pkg/rustc"

setup/rust:
	@echo "$@: TODO: Support https://tracker.debian.org/pkg/rustup"
	curl --insecure  --proto '=https' --tlsv1.2 -sSf  ${rust_url} | bash -s -- -y --default-toolchain ${RUST_VERSION}
	cat $${HOME}/.cargo/env
	@echo '$@: info: You might like to add ". $${HOME}/.cargo/env" to "$${HOME}/.bashrc"'
	-which rustc
	rustc --version
	cargo --version
	rustc --print target-list
	@echo "$@: TODO: https://github.com/kornelski/cargo-deb/issues/159"
	cargo install --version 1.44.0 --locked cargo-deb
	@echo "$@: TODO: Support stable version from https://releases.rs/ or older"

setup/python:
	python3 --version
	@echo "$@: TODO: https://bugs.debian.org/1094297"
	pip3 --version || echo "warning: Please install pip"
	pip3 install "pybars3" \
		|| pip3 install --break-system-packages "pybars3"

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


git/lfs/prepare:
	[ ! -r .git/lfs/objects ] \
	  || { git lfs version || echo "$@: warning: Please install git-lfs" \
	  && git lfs status --porcelain || git lfs install \
	  && time git lfs pull \
	  && git lfs update || git lfs update --force \
	  && git lfs status --porcelain \
	  && git lfs ls-files \
	  ; }

git/modules/prepare:
	[ ! -r .git/modules ] || git submodule update --init --recursive

git/prepare: git/modules/prepare git/lfs/prepare

configure: ${build_dir}/CMakeCache.txt
	file -E $<

${build_dir}/CMakeCache.txt: CMakeLists.txt
	cmake ${cmake_options}

build: ${build_dir}/CMakeCache.txt
	cmake --build ${<D} \
		|| cat ${build_dir}/CMakeFiles/CMakeOutput.log
	cmake --build ${<D}
.PHONY: build

${build_dir}/%: build
	file -E "$@"

test: ${build_dir}
	ctest --test-dir ${<}

check: test

dist: ${build_dir}
	cmake --build $< --target package
	install -d $</$@
	cp -av ${<}/*.deb $</$@

distclean:
	rm -rf ${build_dir}

prepare: git/prepare

all/default: configure prepare build test dist
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
