# Unify Home Assistant Bridge

The Unify Home Assistant Bridge (UHAB) integrates Unify into Home Assistant.
This allows for easy use and integration of the Unify Framework exposing numerous
devices running on multiple wireless protocols.

## Requirements

The UHAB integrates into an existing Home Assistant environment. To enable the
Unify bridge the components requirements must be met:

* MQTT-Broker
* Home Assistant
* Unify Framework with a Protocol Controller (PC) i.e. ZPC, ZigPC.

## Installation

* Home Assistant and Unify must run on same MQTT broker.
* Copy `uhab` folder to `custom_components` on your HA installation. If
  custom_components does not exist, create it in the root of the HA config
  folder, ie. same level as the `configuration.yaml` file.
* Add `unify:` in the `configuration.yaml` file on any line.
* Modify: line [__init__.py](./__init__.py#L96) to match your MQTT broker.

Word of caution, it is very verbose on log level `_LOGGER.debug`.

## Supported Clusters

Unify is based on the Zigbee Cluster Library. The clusters supported in the
Unify bridge UHAB is listed below.

* OnOff
  * Maps to HA Switch
