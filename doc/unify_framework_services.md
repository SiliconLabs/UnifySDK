# Framework Services

The following services facilitate different functionalities in cooperation with
the Protocol Controller(s).



```{toctree}
---
maxdepth: 2
hidden:
---
../applications/upvl/readme_user.md
```

- [The UPVL](../applications/upvl/readme_user.md) (Unify Provisioning List) serves the SmartStart Provisioning
list to perform SmartStart Security 2 (S2) inclusions and maintains the
ucl/SmartStart MQTT topic.

```{toctree}
---
maxdepth: 2
hidden:
---
../applications/gms/readme_user.md
```
- [GMS](../applications/gms/readme_user.md) (Group Manager) manages groups and bookkeeping between
protocol controllers and also publishes group state changes to the ucl/by-group
MQTT topic.


```{toctree}
---
maxdepth: 2
hidden:
---
../applications/nal/readme_user.md
```
- [NAL](../applications/nal/readme_user.md) (Name and Location service) is a helper MQTT component that allows for book-keeping of text names and locations that have been assigned. This functionality allows IoT Services to assign and read back a Name and a Location for each node/endpoint.

```{toctree}
---
maxdepth: 2
hidden:
---
../applications/image_provider/readme_user.md
```
- [OTA Image Provider](../applications/image_provider/readme_user.md) announces OTA
images available in OTA storage and publishes OTA binary on request.

```{toctree}
---
maxdepth: 2
hidden:
---
../applications/upti_cap/readme_user.md
```

- [UPTICap (upti_cap)](../applications/upti_cap/readme_user.md) is an application to communicate with Silicon Labs WSTK adapters. The adapters capture data on the debug channel and publish the captured data as MQTT messages. Application provided strictly for
test and demonstration purposes only and is not suitable for production.

```{toctree}
---
maxdepth: 2
hidden:
---
../applications/upti_writer/readme_user.md
```

- [UPTIWriter (upti_writer)](../applications/upti_writer/readme_user.md) is an application to receive trace packages captured
with _UPTI_CAP_ application and save them to a file in [Network Analyzer](https://docs.silabs.com/simplicity-studio-5-users-guide/latest/ss-5-users-guide-tools-network-analyzer/) compatible format. Application provided strictly for test and demonstration purposes only
and is not suitable for production.