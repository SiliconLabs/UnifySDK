#!/bin/bash
#runDockerUIC.sh - this is a helper script for the docker image used to build UIC projects

#GSDK_LOCATION is the directory where the Gecko SDK is located,
#required if intending to build the Zigbee Protocol Controller, but can otherwise be left blank

#UIC_REPO is the directory where the Unify source code is located
#this should always be filled in
GSDK_LOCATION=""
UIC_REPO=""

docker run -it --rm --env GSDK_LOCATION -v ${GSDK_LOCATION}:${GSDK_LOCATION}  -v ${UIC_REPO}:${UIC_REPO} zpc bash
