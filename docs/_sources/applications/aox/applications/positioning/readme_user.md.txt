# AoX Positioning User Guide

This user guide will guide the reader on how to install, configure and run the
Positioning IoT Service.

Readers of this document should be familiar with the [AoXPC](../aoxpc/readme_user.md),
because the position estimation service is based on the angle data provided by
the AoXPC instances.

## Operation

The positioning service starts automatically as a systemd service. For more
information on installation and systemd services, see [Unify User guide](../../../../doc/unify_readme_user.md).

The service detects the locators connected to the system automatically.
Locators need to be configured as described in the [AoXPC User Guide](../aoxpc/readme_user.md).
The locator configuration is considered valid if the `PositionAndOrientationValid`
attribute is set to `true`.

Once a locator is configured, there is no way to exclude it from the position
estimation. Only one positioning service is supported per MQTT broker.

Asset tags are presented as individual nodes on the MQTT network by the
positioning service. The estimated position of the asset tag is presented as an
AoXPositionEstimation cluster attribute, as follows.

```console
ucl/by-unid/<TAG_UNID>/ep0/AoXPositionEstimation/Attributes/Position/Reported {
  "value": {
    "ApplicationId": "positioning_4154",
    "DeviationX": 3.1297969818115234,
    "DeviationY": 0.6999226212501526,
    "DeviationZ": 3.8446271419525146,
    "Sequence": 47021,
    "X": 0.8519021272659302,
    "Y": 0.000349800189724192,
    "Z": 0.8335742950439453
  }
}
```

### Position Estimation with One Locator

The positioning service allows to estimate position even with one locator.
To achieve this, RSSI based distance estimation is used. If the received signal
strength of the incoming signal is altered for any other reason than the
distance between the TX and RX itself, it will lead to incorrect distance
measurement and it will lead to incorrect position estimates.

Therefore, using multiple locators is advised for accurate position estimation.

## Configuration

The positioning service can be configured with the same JSON config file as the
AoXPC. The path of the config file can be specified with the
`--positioning.config` CLI option.

These are the configuration parameters that are relevant for the
positioning service, with their default values:

```json
{
  "id":"positioning_<PID>",
  "estimationModeLocation":"SL_RTL_LOC_ESTIMATION_MODE_THREE_DIM_HIGH_ACCURACY",
  "validationModeLocation":"SL_RTL_LOC_MEASUREMENT_VALIDATION_MEDIUM",
  "estimationIntervalSec":0.02,
  "locationFiltering":true,
  "locationFilteringWeight":0.1,
  "numberOfSequenceIds":6,
  "maximumSequenceIdDiffs":20,
}
```

Let's see these parameters in detail.
### id

This parameter controls the `ApplicationId` field of the `Position` attribute.
The default value is derived from the process ID. It can be replaced with any
string.

### estimationModeLocation

Estimation mode used by the position estimation algorithm.

For more details, see [sl_rtl_loc_set_mode](https://docs.silabs.com/rtl-lib/latest/group-sl-rtl-loc#gacf5978113a013aa36cbdb85bfc937aed).

### validationModeLocation

The positioning service implements an angle correction mechanism for a deviating
locator whose angle reports are far off the expected one, and thus is not
pointing to the same direction where all other locators see the tag. The
correction feedback provides the locator with the information in which direction
it is supposed to see the tag. This helps locators that locked up on a
reflection signal which is stronger than the one coming in the line-of sight
direction.

The `validationModeLocation` parameter determines the sensitivity of the angle
correction mechanism.

For more details, see [sl_rtl_loc_set_measurement_validation](https://docs.silabs.com/rtl-lib/latest/group-sl-rtl-loc#ga424290857cf7416aef48d72ed31d84a2).

### estimationIntervalSec

Estimation interval in seconds. This value should approximate the time interval
between two consecutive CTEs received from an asset tag.

For more details, see [sl_rtl_loc_process](https://docs.silabs.com/rtl-lib/latest/group-sl-rtl-loc#gaa2a04c7e40ec23d2451cd63d0a6024db).

### locationFiltering, locationFilteringWeight

An optional filter can be applied on the position coordinates, `X`, `Y` and `Z`
to reduce noise. Filter weight ranges from 0 to 1.

For more details, see [sl_rtl_util_filter](https://docs.silabs.com/rtl-lib/latest/group-sl-rtl-util#ga41610abd68f01bc6871e084fc28ed000).

### numberOfSequenceIds, maximumSequenceIdDiffs

The CTE signals can be identified by their sequence ID which is a monotonic
increasing number. When the positioning service receives an angle report
estimated from that CTE, it knows
- the transmitter of the CTE (asset tag)
- the receiver of the CTE (locator)
- the sequence ID of the CTE.

Ideally, a CTE signal is processed by all of the locators in the AoA system, so
the positioning service simply has to wait for all angle reports from all
locators with a given sequence ID. Once all angle reports have arrived, the
positioning can start the position estimation. But what happens if a CTE is
missed by a locator, or a locator is temporarily unavailable?

The `angle_queue` component is responsible for collecting the angle reports that
belong to the same CTE (i.e., have the same sequence ID) and go on with position
estimation even under suboptimal conditions.
`numberOfSequenceIds` and `maximumSequenceIdDiffs` can be used to tweak the
behavior of the `angle_queue` component.

The `numberOfSequenceIds` tells how many sequence IDs shall be tracked maximum
for a given asset tag. If `numberOfSequenceIds` is exceeded, the angle reports
with the oldest sequence ID are deleted. This parameter also determines the
number of angle reports the positioning has to wait for before starting the
position estimation. The expected angle count depends on the "age" of
the sequence ID in a linear way. This means, Only 2 angle reports are expected
for the oldest sequence ID, for the newest sequence ID all locators are expected
to send an angle report. In the following example, we have 7 locators,
`numberOfSequenceIds` is 6.

| Slot            |  0  |  1  |  2  |  3  |  4  |  5  |
|-----------------|-----|-----|-----|-----|-----|-----|
| Sequence ID     | 101 | 102 | 103 | 104 | 105 | 106 |
| Angles expected |   2 |  3  |  4  |  5  |  6  |  7  |

`maximumSequenceIdDiffs` sets another limit on the sequence IDs. If the sequence
ID difference between the oldest and the newest angle report exceeds this limit,
then the angle reports with the oldest sequence ID are deleted.
