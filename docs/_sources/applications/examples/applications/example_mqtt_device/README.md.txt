# Example MQTT Device

This applicate simulates an MQTT device having all possible UCL capabilities
using the Unify Library.

## Build the application

Use the following define with CMake to build this application: `-DBUILD_EMD=ON`.
For example:

```console
pi@unify:/build $ cmake -GNinja .. -DCMAKE_BUILD_TYPE=RelWithDebugInfo -DBUILD_EMD=ON
[...]
-- Example MQTT Device (EMD) will build
pi@unify:/build $ ninja
```

## Start the application

It is important to configure the path of the UAM files for the basic cluster
functionalities to work.

```console
pi@unify:/build $ ./applications/examples/applications/example_mqtt_device/emd --mqtt.host localhost --log.level debug --emd.datastore_file emd.db --mapdir ../applications/examples/applications/example_mqtt_device/components/emd_attribute_mapper/rules/
```

Use the console to create new devices/endpoints.

```console
EMD>help
==================================================
Unify Command line interface Help:
==================================================
add_endpoint Add an endpoint under a UNID in the Attribute Store
add_unid Add a UNID in the Attribute Store
# --- SNIP ---
remove_endpoint Removes an endpoint under a UNID from the Attribute Store
remove_unid Removes a UNID from the Attribute Store
```

For example, add a device with UNID 1 and endpoint 0. Endpoint 0 will also be
created if you specify any other endpoint number.

```console
EMD>add_endpoint 1,0
# Attributes are created and published here.
```

Observe on the MQTT publications. Devices will appear Online with all
possible attributes and commands supported by the Unify DotDot Attribute
Store library.

Commands will affect the desired values. The application will wait the value
configured on the MaximumCommandDelay before it aligns the reported values.

The Unify Attribute Store commands are available via the console, if you want
to update values that are not updatable via MQTT.

It is possible to Remove UNIDs using the MQTT state topic, it will simulate
a node removal.

It is also possible to simulate a node interview. The interview will take the
value configured in the INTERVIEW_TIME define.