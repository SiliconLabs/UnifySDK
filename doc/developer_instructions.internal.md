# Unofficial Readme

This is an UN-OFFICIAL guide for running the whole setup to run ZPC, UPVL, ZigPC,
and Developer GUI on Ubuntu 20.04 (it should also work for Debian Buster).
Debian Buster armhf is the officially supported platform.

## Build Unify Host SDK

First get the Unify Host SDK source code by cloning Unify repo.

~~~ bash
git clone ssh://git@stash.silabs.com/uic/uic.git
~~~

~~~ bash
git submodule update --init --recursive
~~~

Then follow the internal [Build instructions](./build_instructions.internal.md).

## Running Unify

After having build and compiled the Unify Host SDK  there are several applications
one can run. Some are dependent on others for full functionality.

### Download and install mosquitto broker

~~~ bash
apt install -y curl gnupg2
curl -s http://repo.mosquitto.org/debian/mosquitto-repo.gpg.key | apt-key add -
curl -s  http://repo.mosquitto.org/debian/mosquitto-buster.list > /etc/apt/sources.list.d/mosquitto-buster.list
apt update
apt install -y mosquitto libmosquitto1
~~~

Make sure your Mosquitto MQTT broker is running:

~~~ bash
systemctl status mosquitto
~~~

If it is not running:

~~~ bash
systemctl start mosquitto
~~~

If you encounter problems with running the service, have a look at the logs:

~~~ bash
journalctl -u mosquitto
~~~

Note: this should start Mosquitto on localhost at port 1883. Unify applications should connect to it by default.

### Running Z-Wave Protocol Controller

Application dependencies for full functionality:

* Mosquitto broker
* UPVL

Running the ZPC:

~~~ bash
./applications/zpc/zpc --zpc.datastore_file /tmp/datastore.db --zpc.serial /dev/ttyUSB0
--mapdir ./applications/zpc/components/dotdot_mapper/rules --mqtt.host 0.0.0.0
--mqtt.port 1883
~~~

TIP: you can do following to make bash auto-complete ZPC command line

~~~ bash
source applications/zpc/scripts/bash-completion/zpc
~~~

### Running the ZigBee Protocol Controller

Application dependencies for full functionality:

* Mosquitto broker
* UPVL

Running the ZigPC:

~~~ bash
./applications/zigpc/zigpc --zigpc.serial /dev/ttyUSBX --mqtt.host 0.0.0.0
--mqtt.port 1883 --zigpc.datastore_file zigpc.db
~~~

### Running Unify Framework Provisioning List

Application dependencies for full functionality:

* Mosquitto broker
* libuic_log.so

The UPVL is linked to **libuic_log.so** so for running the UPVL binary one needs
to have this library installed. By packaging Unify to debian package on can
install **libuic** library.

~~~ bash
uic/build $ ninja deb # Will package Unify
uic/build $ sudo apt-get install ./libuic_x.x.x-<patch>_x86_64.deb
~~~

After installing libuic library the binary can be for UPVL can be executed:

~~~ bash
uic/build $ ./applications/upvl/x86_64-unkown-linux-gnu/debug/uic-upvl
~~~

### Running Unify Framework Developer GUI

Application dependencies for full functionality:

* Mosquitto broker

After building the Unify Host SDK the two binaries are always cross compiled to
Linux. Thus there are two different ways starting the DevGUI depending on your
OS.

**For linux (arm/x86_64):**
After building Unify there is two binaries for Developer GUI that needs to be run.

1. First the **dev-gui-api** for MQTT connection

  ~~~ bash
  uic/build $ ./applications/dev_ui/dev_gui/dev-gui-api
  ~~~

2. Second the client it self actually running the UI

  ~~~ bash
  uic/build $ ./applications/dev_ui/dev_gui/dev-gui-client
  ~~~

**MacOS/Windows**

To start the UI on Windows or Mac one needs to run the two following commands
instead to start the application (Note: the startup is slow approx ~60 seconds):

1. In the first terminal run:

  ~~~ bash
  uic/build/applications/dev_ui/dev_gui/ $ npm run start-api
  ~~~

2. In the second terminal run:

  ~~~ bash
  uic/build/applications/dev_ui/dev_gui/ $ npm run start
  ~~~

This will start the developer GUI locally on your Windows or MacOS machine.

## Debugging core dumps

In the event that SQA encounters a problem which causes an application to crash or
getting stuck we can learn from a code dump. A raspberry can be setup to automatically
generate a core dump by running the command

```bash
ulimit -S -c 0
./<my application>
```

a system wide setup can be done by modifying the file _/etc/security/limits.conf_.

To collect a core dump of a running application use attach gdb to the running process
and run the _generate-core-file_.

```bash
gdb /usr/bin/myprgram <pid>
(gdb) generate-core-file
```

The simplest way to analyse a core file is to use gdb on the same machine / raspberry like

```bash
gdb myprgram <core-file>
```

But a core file can also be analyzed in a docker container. But it is important to
have both the application and all shared libraries. The shared libraries can be loaded
with the command _set solib-search-path_

```bash
user@ac449947dd30:/uic/UIC-1650$ gdb-multiarch zpc core.18915
(gdb) set solib-search-path libuic
Reading symbols from /uic/UIC-1650/libuic/libuic_main.so...done.
Reading symbols from /uic/UIC-1650/libuic/libuic_attribute_mapper.so...done.
Reading symbols from /uic/UIC-1650/libuic/libuic_attribute_utils.so...done.
Reading symbols from /uic/UIC-1650/libuic/libuic_ota.so...done.
Reading symbols from /uic/UIC-1650/libuic/libuic_dotdot_mqtt.so...done.
Reading symbols from /uic/UIC-1650/libuic/libuic_smartstart_management.so...done.
Reading symbols from /uic/UIC-1650/libuic/libuic_attribute_resolver.so...done.
Reading symbols from /uic/UIC-1650/libuic/libuic_stdin.so...done.
Reading symbols from /uic/UIC-1650/libuic/libuic_mqtt.so...done.
Reading symbols from /uic/UIC-1650/libuic/libuic_mqtt_client.so...done.
Reading symbols from /uic/UIC-1650/libuic/libuic_mqtt_wrapper.so...done.
Reading symbols from /uic/UIC-1650/libuic/libuic_attribute_store.so...done.
Reading symbols from /uic/UIC-1650/libuic/libuic_datastore.so...done.
Reading symbols from /uic/UIC-1650/libuic/libuic_main_fd.so...done.
Reading symbols from /uic/UIC-1650/libuic/libuic_contiki.so...done.
Reading symbols from /uic/UIC-1650/libuic/libuic_gbl_interface.so...done.
Reading symbols from /uic/UIC-1650/libuic/libuic_log.so...done.
Reading symbols from /uic/UIC-1650/libuic/libuic_config.so...done.
Reading symbols from /uic/UIC-1650/libuic/libuic_contiki_platform.so...done.
(gdb) bt
#0  0xb6443664 in check_dst_limits_calc_pos_1 (mctx=0x76b000, boundaries=0, subexp_idx=142, from_node=-1096131432, bkref_idx=29) at regexec.c:1916
#1  0xb6f6bff2 in uic_main_loop () at ../components/uic_main/src/uic_main_loop.c:100
#2  0xb6f6bee6 in uic_main (fixt_app_setup=<optimized out>, fixt_app_shutdown=0x9663858, argc=157695040, argv=<optimized out>, version=0x6d02e4 "1.1.0") at ../components/uic_main/src/uic_main.c:43
#3  0x004a22b8 in main (argc=1, argv=<optimized out>) at ../applications/zpc/main.c:209
```

Note that is may be that the libc debug symbol in the docker does not fit
the version running on the raspberry pi. So gdb may show wrong symbol names
for libc functions. Like above, the application is not really running check_dst_limits_calc_pos_1

```
(gdb) up
#1  0xb6f6bff2 in uic_main_loop () at ../components/uic_main/src/uic_main_loop.c:100
(gdb) layout asm
```

Reveals that we are more likely to be in a select loop.

Variables can now be inspected:

```
(gdb) p tx_queue
$2 = {<std::multiset<zwave_tx_queue_element, queue_element_qos_compare, std::allocator<zwave_tx_queue_element> >> = std::multiset with 0 elementsPython Exception <class 'ValueError'> Cannot find type class std::multiset<zwave_tx_queue_element, queue_element_qos_c
ompare, std::allocator<zwave_tx_queue_element> >::_Rep_type:
, zwave_tx_session_id_counter = 1906}

(gdb) p queue_flush_ongoing
$1 = true
(gdb) p state
$3 = {on_send_data_complete = 0x0, user = 0x0, transmission_ongoing = false, parent_session_id = 0x771, remote_node_id = 0, fast_track = false, awaiting_nif = false, emergency_timer = {timer = {start = 156665575, interval = 65000}, next = 0x0, p = 0x0}}
(gdb) p current_tx_session_id
$4 = (zwave_tx_session_id_t) 0x770
```
