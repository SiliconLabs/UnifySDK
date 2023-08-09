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
| libunify           | 15427 kB |                      N/A |
| uic-aoxpc          |  1507 kB |                 17872 kB |
| uic-zigpc          |  1485 kB |                 17332 kB |
| uic-zpc            |  5061 kB |                 24888 kB |
| uic-gms            |  1579 kB |                 16160 kB |
| uic-nal            |  1620 kB |                 17324 kB |
| uic-positioning    |  1420 kB |                 16816 kB |
| uic-upti-cap       |  1624 kB |                 16276 kB |
| uic-upti-writer    |  1608 kB |                 16168 kB |
| uic-image-provider |  1579 kB |                 16440 kB |
| uic-upvl           |  1628 kB |                 17392 kB |
| uic-dev-gui        | 48089 kB |                 47440 kB |
| zigbeed            |  4019 kB |                  3108 kB |
| Z3Gateway          |   621 kB |                      TBD |
| uic-cpcd           |   290 kB |                   772 kB |
| uic-cpc-hci-bridge |    10 kB |                      TBD |

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
