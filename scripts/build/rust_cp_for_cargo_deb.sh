#!/bin/sh
if [ "${LIB_BUILD_TYPE}" = "debug" ]; then
    if [ ! -d "${deb_release_dir}" ]; then 
        mkdir -p "${deb_release_dir}"
    fi
    cp -fR "${binary_dir}" "${deb_release_dir}"
fi