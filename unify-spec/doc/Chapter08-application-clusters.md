# Application-Level Clusters

This chapter contains cluster definitions of application level functionality
that is unique to the Unify Controller Language. This is an addition to the
clusters already defined in the ZCL.

## Location Cluster

This section defines how to represent Angle-of-X (e.g, Angle of Arrival) IQ
report, Angle, and Position data in the Unify SDK.

The AoX data must be published by an AoX Protocol Controller to an MQTT Broker.
Typically, an AoX Protocol Controller only supports the Location cluster along
with a minimal ProtocolController/NetworkManagement topic. See
[Special Requirements for BLE AoX Protocol Controllers](doc/Chapter04-scenario-mappings.md)
for details.

### ClusterRevision Attribute

The Location Cluster revision MUST be set to 1.

```mqtt
ucl/by-unid/<LOCATOR_UNID>/ep0/Location/Attributes/ClusterRevision/Desired {"value": 1}
ucl/by-unid/<LOCATOR_UNID>/ep0/Location/Attributes/ClusterRevision/Reported {"value": 1}
```

### SupportedCommands topic

The Location Cluster does not require any command to be operated.
Therefore, the SupportedCommands topic MAY be omitted. It is still recommended
to publish an empty list of commands for this topic.

**MQTT Topic:** `ucl/by-unid/<LOCATOR_UNID>/ep0/Location/SupportedCommands`

**Payload:**

```json
{ "value": [] }
```

### Tag UNIDs

Tag UNIDs must indicate the BLE address type in addition to the BLE address. The
generic format is `ble-<ADDRESS_TYPE>-<BLE_ADDRESS>`. A list of potential address
types can be seen in the section titled Unique Identifiers here: AoX Locator
Protocol Converter Specification. Static random addresses can be abbreviated
sr and public device addresses pd.

An example of a public device address TAG_UNID is `ble-pd-DEADBEEF1234`.

### IQ Report

Whenever an IQ report for a tag is available, the AoX PC must publish the
IQReport topic.

**MQTT topic:** `ucl/by-unid/<LOCATOR_UNID>/ep0/Location/Attributes/IQReport/<TAG_UNID>/Reported`

**Payload:**

```json
{
    "channel": 13,
    "rssi": -50,
    "sequence": 123,
    "samples": [23, 105, 106, -10, 2, -108, ...]
}
```
### Angle

Depending on the configuration of the AoX PC, angle data is sent instead of the IQ
reports.

Whenever angle data for a tag is available, the AoX PC must publish the Angle
topic.

**MQTT topic:** `ucl/by-unid/<LOCATOR_UNID>/ep0/Location/Attributes/Angle/<TAG_UNID>/Reported`

**Payload:**

```json
{
    "azimuth": 60.0,
    "elevation": 120.0,
    "distance": 2.5,
    "quality": 0,
    "sequence": 123
}
```

### Position

Whenever position data for a tag is available, the multi-locator must publish the
Position topic.

**MQTT topic:** `ucl/by-unid/<MULTILOCATOR_UNID>/ep0/Location/Attributes/Position/<TAG_UNID>/Reported`

**Payload:**

```json
{
    "x": 1.0,
    "y": 2.3,
    "z": 3.2,
    "sequence": 123
}
```
