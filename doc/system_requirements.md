# System Requirements

The Unify SDK is at the moment only implemented for the Linux OS. The table
below shows the resource usage of the individual components measured on arm64
release binaries. No debug symbols included. RAM figures are indicative for
applications without any data added (i.e. initial memory usage). The measurement
used for the RAM figures is the programs Resident Set Size (RSS) measured by
`ps` or `smem`. Bear in mind that the RAM measurement for each application also
includes memory used by shared libraries, thereby the actual memory consumed
when running multiple Unify applications that use the same shared library
(libunify and various system libraries) is lower than the sum of this table.

> * Release binaries are compiled with `-O2 -s`
> * RAM size = Resident Set Size (RSS)

| Component          |    Flash |                RAM (RSS) |
| ------------------ | -------: | -----------------------: |
| libunify           | 16773 kB |                      N/A |
| uic-zpc            |  5621 kB |                 26048 kB |
| uic-gms            |  6519 kB |                 18152 kB |
| uic-nal            |  6609 kB |                 19224 kB |
| uic-upti-cap       |  6610 kB |                 18080 kB |
| uic-upti-writer    |  6591 kB |                 18088 kB |
| uic-image-provider |  6523 kB |                 17980 kB |
| uic-upvl           |  6616 kB |                 19200 kB |
| uic-dev-gui        | 48130 kB |                 48676 kB |

The SDK depends on the following Debian packages:

* ipset
* libavahi-client3
* libboost-atomic1.74.0
* libboost-chrono1.74.0
* libboost-date-time1.74.0
* libboost-filesystem1.74.0
* libboost-log1.74.0
* libboost-program-options1.74.0
* libboost-regex1.74.0
* libboost-system1.74.0
* libboost-thread1.74.0
* libipset13
* libmbedcrypto3
* libmbedtls12
* libmbedx509-0
* libyaml-cpp0.6
* socat
* libmosquitto1
