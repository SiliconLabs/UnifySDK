#!/bin/bash
#/******************************************************************************
# * # License
# * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
# ******************************************************************************
# * The licensor of this software is Silicon Laboratories Inc. Your use of this
# * software is governed by the terms of Silicon Labs Master Software License
# * Agreement (MSLA) available at
# * www.silabs.com/about-us/legal/master-software-license-agreement. This
# * software is distributed to you in Source Code format and is governed by the
# * sections of the MSLA applicable to Source Code.
# *
# *****************************************************************************/

# usage:
#    merge_libs.h <target_lib>.a <additional_lib>.a
#
# This script merges the additional lib into the target lib
# Duplicate symbols inside the additional lib are omitted.

target_lib=$1
additional_lib=$2

if [ -z "$target_lib" ] || [ -z "$additional_lib" ]; then
    exit 1
fi

temp=$(mktemp -d)
mkdir -p "$temp"
cd "$temp" || exit

${CMAKE_AR} -x "$additional_lib"
${CMAKE_AR} -x "$target_lib"
${CMAKE_AR} -rc "$target_lib" *.o
