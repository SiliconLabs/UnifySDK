# System Requirements

The Unify Host SDK is at the moment only implemented for the Linux OS. The table
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
| libunify           | 12944 kB |                      N/A |
| uic-aoxpc          |  1460 kB |                 10500 kB |
| uic-zigpc          |  1828 kB |                 15000 kB |
| uic-zpc            |  4883 kB |                    23 kB |
| uic-gms            |  1542 kB |                 15932 kB |
| uic-nal            |  1590 kB |                 17356 kB |
| uic-positioning    |  4256 kB |                 16300 kB |
| uic-upti-cap       |  1590 kB |                 16036 kB |
| uic-upti-writer    |  1570 kB |                  8700 kB |
| uic-image-provider |  1542 kB |                 15944 kB |
| uic-upvl           |  1602 kB |                 17296 kB |
| uic-dev-gui        | 43680 kB |                 49404 kB |
| zigbeed            |   496 kB |                  1100 kB |
| Z3Gateway          |   608 kB |                   600 kB |
| uic-cpcd           |   275 kB |                  2200 kB |
| uic-cpc-hci-bridge |    10 kB |                   TBD kB |

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
