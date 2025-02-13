# Emulated End Device (EED)

## Building EED

EED is enabled in build by default to explicitly enabled it use the following define with CMake to build this application: `-DBUILD_EED=ON`.
For example:

```console
pi@unify:/build $ cmake -GNinja .. -DCMAKE_BUILD_TYPE=RelWithDebugInfo -DBUILD_EED=ON
[...]
-- Emulated End Device (EED) will build
pi@unify:/build $ ninja
```

## Things to note
Devices will appear Online with all possible attributes and commands supported by the Unify DotDot Attribute Store library for the configured clusters.

Commands will affect the desired values. The application will wait for 90% of the value
configured on the MaximumCommandDelay before it aligns the reported values.

It is possible to Remove UNIDs using the MQTT state topic, it will simulate
a node removal.

It is also possible to simulate a node interview. The interview will take the
value configured in the INTERVIEW_TIME define.