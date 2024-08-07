#!/bin/bash
set -e
set -o pipefail

# Build docker image
if [ "$#" -lt 2 ]; then
	echo "Illegal number of arguments." >&2
  echo ""
	echo "Usage: $0 arch tag [docker_args]+"
	echo "  arch:        arch to send to docker as build-arg ARCH"
	echo "  tag:         tag that docker image will be build with"
	echo "  docker_args: addition arguments passed to docker build command (optional)"
	echo ""
	echo "Example: $0 arm64 uic_arm64"
	exit 1
fi

arch=$1
tag=$2

docker_args=""
if [ "$#" -gt 2 ]; then
	docker_args="${@:3}"
fi

echo "info: Make sure that LFS assets are in tree before building"
git lfs version || echo "warning: Please install git-lfs"
count=$(git lfs status --porcelain  | grep '^D' | wc -l  || echo 0)
if [ "0" != "$count" ] ; then
    echo "warning: Attempt to fetch LFS assets from online"
    git lfs pull
fi

# Build docker image
docker_build_command="docker build --network host -t ${tag} --build-arg ARCH=${arch} --build-arg USER_ID=`id -u` --build-arg GROUP_ID=`id -g` ${docker_args} `dirname $0`"
echo "=== Building Docker image:"
echo $docker_build_command
$docker_build_command

