""" 
Copyright 2022 Silicon Laboratories, www.silabs.com

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""

import json
import logging
from typing import final
from .base_unify_entity import BaseUnifyEntity, setup_device_data_structure
from .const import DOMAIN, DEVICES
from homeassistant.components.mqtt.models import ReceiveMessage
from homeassistant.components.sensor import SensorEntity
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity_platform import AddEntitiesCallback
from homeassistant.const import (
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_TEMPERATURE,
    PERCENTAGE,
    TEMP_CELSIUS,
)

_LOGGER = logging.getLogger(__name__)


async def async_setup_platform(
        hass: HomeAssistant,
        config,
        async_add_entities: AddEntitiesCallback,
        discovery_info=None):
    if discovery_info is None:
        return
    unid = discovery_info["unid"]
    endpoint = discovery_info["endpoint"]
    setup_device_data_structure(unid, endpoint, hass.data[DOMAIN][DEVICES])
    devices = []
    if "temperaturemeasurement" in discovery_info["clusters"]:
        hass.data[DOMAIN][DEVICES][unid][endpoint]["temperaturemeasurement"] = UnifyTemperature(
            hass, unid, endpoint)
        devices.append(hass.data[DOMAIN][DEVICES][unid]
                       [endpoint]["temperaturemeasurement"])
    if "powerconfiguration" in discovery_info["clusters"]:
        hass.data[DOMAIN][DEVICES][unid][endpoint]["powerconfiguration"] = UnifyBattery(
            hass, unid, endpoint)
        devices.append(hass.data[DOMAIN][DEVICES][unid]
                       [endpoint]["powerconfiguration"])
    if "scenes" in discovery_info["clusters"]:
        hass.data[DOMAIN][DEVICES][unid][endpoint]["scenes"] = UnifyScene(
            hass, unid, endpoint)
        devices.append(hass.data[DOMAIN][DEVICES][unid][endpoint]["scenes"])
    async_add_entities(devices)


class UnifyTemperature(SensorEntity, BaseUnifyEntity):
    """Representation of a Unify Temperature Sensor."""

    def __init__(self, hass, unid, endpoint):
        BaseUnifyEntity.__init__(self, hass, unid, endpoint)
        _LOGGER.info("UnifyTemperature: Init")
        self._native_value = 0
        self._device_class = DEVICE_CLASS_TEMPERATURE
        self._native_unit_of_measurement = TEMP_CELSIUS
        self._state_class = "measurement"

    @final
    async def async_added_to_hass(self):
        await self.async_subscribe(
            f"ucl/by-unid/{self._unid}/{self._ep}/TemperatureMeasurement/Attributes/MeasuredValue/Reported",
            self._on_message_state)

    def _on_message_state(self, msg: ReceiveMessage):
        _LOGGER.info("Temp: State change")
        parsed_msg = json.loads(msg.payload)
        temp = int(parsed_msg["value"]) / 100
        _LOGGER.info(
            "UnifyTemperature: state changed for %s to %s Payload %s ",
            self._name,
            msg.topic,
            parsed_msg,
        )
        self._native_value = temp
        _LOGGER.info("UnifyTemperature: state detected %s", self._native_value)
        try:
            self.schedule_update_ha_state(False)
            _LOGGER.info("UnifyTemperature: HA State updated: %s",
                         self._native_value)
        except Exception as err:
            _LOGGER.warning(
                "UnifyTemperature: Exception on State Update: %s", err)

    @ property
    def unique_id(self):
        """Force update."""
        entity_unid = f"temperature_{self._unid}_{self._ep}"
        entity_unid.replace("-", "_")
        return entity_unid

    @ property
    def device_class(self):
        """Return the class of this device, from component DEVICE_CLASSES."""
        return self._device_class

    @ property
    def native_value(self):
        """Return the state of the sensor."""
        return self._native_value

    @ property
    def native_unit_of_measurement(self):
        """Return the unit of measurement of this entity, if any."""
        return self._native_unit_of_measurement


class UnifyScene(SensorEntity, BaseUnifyEntity):
    """Representation of a Unify Scene."""

    def __init__(self, hass, unid, endpoint):
        BaseUnifyEntity.__init__(self, hass, unid, endpoint)
        self._native_value = ""
        self._device_class = ""
        self._native_unit_of_measurement = ""
        self._state_class = "measurement"

    @final
    async def async_added_to_hass(self):
        await self.async_subscribe(
            f"ucl/by-unid/{self._unid}/{self._ep}/Scenes/GeneratedCommands/RecallScene",
            self._on_message_state)

    def _on_message_state(self, msg: ReceiveMessage):
        parsed_msg = json.loads(msg.payload)
        _LOGGER.info(
            "UnifyScene: state changed for %s to %s Payload %s ",
            self._name,
            msg.topic,
            parsed_msg,
        )
        self._native_value = parsed_msg["SceneID"]
        _LOGGER.debug("UnifyScene: state detected %s", self._native_value)
        try:
            self.schedule_update_ha_state(True)
            _LOGGER.debug("UnifyScene: HA State updated: %s",
                          self._native_value)
        except Exception as err:
            _LOGGER.warning("UnifyScene: Exception on State Update: %s", err)

    @ property
    def unique_id(self):
        """Return the device unique id"""
        unique_id = f"scene_{self._unid}_{self._ep}"
        unique_id.replace("-", "_")
        return unique_id

    @ property
    def device_class(self):
        """Return the class of this device, from component DEVICE_CLASSES."""
        return self._device_class

    @ property
    def native_value(self):
        """Return the state of the sensor."""
        return self._native_value

    @ property
    def native_unit_of_measurement(self):
        """Return the unit of measurement of this entity, if any."""
        return self._native_unit_of_measurement


class UnifyBattery(SensorEntity, BaseUnifyEntity):
    """Representation of a Unify Battery Sensor."""

    def __init__(self, hass, unid, endpoint):
        _LOGGER.info("UnifyBattery: Init")
        BaseUnifyEntity.__init__(self, hass, unid, endpoint)
        self._native_value = 100
        self._device_class = DEVICE_CLASS_BATTERY
        self._native_unit_of_measurement = PERCENTAGE
        self._state_class = "measurement"

    @final
    async def async_added_to_hass(self):
        await self.async_subscribe(
            f"ucl/by-unid/{self._unid}/{self._ep}/PowerConfiguration/Attributes/BatteryPercentageRemaining/Reported",
            self._on_message_state)

    def _on_message_state(self, msg: ReceiveMessage):
        try:
            parsed_msg = json.loads(msg.payload)
            _LOGGER.info(
                "UnifyBattery: state changed for %s to %s Payload %s ",
                self._name,
                msg.topic,
                parsed_msg,
            )
            self._native_value = parsed_msg["value"]
            _LOGGER.info("UnifyBattery: state detected %s", self._native_value)

            self.schedule_update_ha_state(False)
            _LOGGER.info("UnifyBattery: HA State updated: %s",
                         self._native_value)
        except Exception as err:
            _LOGGER.info("UnifyBattery: Exception on State Update: %s", err)

    @ property
    def unique_id(self):
        """Return the device unique id"""
        unique_id = f"battery_{self._unid}_{self._ep}"
        unique_id.replace("-", "_")
        return unique_id

    @ property
    def device_class(self):
        """Return the class of this device, from component DEVICE_CLASSES."""
        return self._device_class

    @ property
    def native_value(self):
        """Return the state of the sensor."""
        return self._native_value

    @ property
    def native_unit_of_measurement(self):
        """Return the unit of measurement of this entity, if any."""
        return self._native_unit_of_measurement
