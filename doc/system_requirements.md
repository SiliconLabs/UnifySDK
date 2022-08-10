# System Requirements

The Unify Host SDK is at the moment only implemented for the Linux OS. The table
below shows the resource usage of the individual components measured on armhf
release binaries. No debug symbols included. RAM figures are indicative for
applications without any data added (i.e. initial memory usage). The measurement
used for the RAM figures is the programs Resident Set Size (RSS) measured by
`ps` or `smem`. Bear in mind that the RAM measurement for each application also
includes memory used by shared libraries, thereby the actual memory consumed
when running multiple Unify applications that use the same shared library
(libuic and various system libraries) is lower than the sum of this table.

> * Release binaries are compiled with `-O2 -s`
> * RAM size = Resident Set Size (RSS)

| Component          |    Flash |                RAM (RSS) |
| ------------------ | -------: | -----------------------: |
| libuic             | 10568 kB |                      N/A |
| uic-aoxpc          |  4296 kB |                 10500 kB |
| uic-zigpc          |   740 kB |                 15000 kB |
| uic-zpc            |  2580 kB |                 20000 kB |
| uic-gms            |   412 kB |                  8700 kB |
| uic-nal            |   448 kB |                 10000 kB |
| uic-positioning    |  4256 kB |                 11500 kB |
| uic-upti-cap       |   452 kB |                  8600 kB |
| uic-upti-writer    |   400 kB |                  8700 kB |
| uic-image-provider |   428 kB |                  8500 kB |
| uic-upvl           |   492 kB |                 10000 kB |
| uic-dev-gui        | 30000 kB |                 10000 kB |
| zigbeed            |   496 kB |                  1100 kB |
| Z3Gateway          |   608 kB |                   600 kB |
| cpcd               |   160 kB |                  2200 kB |
| cpc-hci-bridge     |    12 kB |                   TBD kB |

The SDK depends on the following Debian packages:

* ipset
* libavahi-client3
* libboost-atomic1.67.0
* libboost-chrono1.67.0
* libboost-date-time1.67.0
* libboost-filesystem1.67.0
* libboost-log1.67.0
* libboost-program-options1.67.0
* libboost-regex1.67.0
* libboost-system1.67.0
* libboost-thread1.67.0
* libipset11
* libmbedcrypto3
* libmbedtls12
* libmbedx509-0
* libyaml-cpp0.6
* socat
* libmosquitto1
