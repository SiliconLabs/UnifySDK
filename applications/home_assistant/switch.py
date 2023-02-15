""" 
Copyright 2022 Silicon Laboratories, www.silabs.com

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""

"""Unify binary switch on off"""

# Standard packages
import logging
import json
from typing import final

# Integration components
from .base_unify_entity import BaseUnifyEntity, setup_device_data_structure
from .const import DOMAIN, DEVICES

# HA packages
from homeassistant.components.switch import SwitchEntity
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity_platform import AddEntitiesCallback
from homeassistant.components import mqtt
from homeassistant.components.mqtt.models import ReceiveMessage


_LOGGER = logging.getLogger(__name__)


async def async_setup_platform(
        hass: HomeAssistant,
        config,
        async_add_entities: AddEntitiesCallback,
        discovery_info=None):
    unid = discovery_info["unid"]
    endpoint = discovery_info["endpoint"]
    setup_device_data_structure(unid, endpoint, hass.data[DOMAIN][DEVICES])
    hass.data[DOMAIN][DEVICES][unid][endpoint]["switch"] = UnifyOnOff(
        hass, unid, endpoint)
    device = hass.data[DOMAIN][DEVICES][unid][endpoint]["switch"]
    async_add_entities([device])


class UnifyOnOff(SwitchEntity, BaseUnifyEntity):
    """Representation of a Unify Switch."""
    _attr_has_entity_name = True
    _attr_name = None

    def __init__(self, hass, unid, endpoint):
        self._is_on = False
        BaseUnifyEntity.__init__(self, hass, unid, endpoint)

    @final
    async def async_added_to_hass(self) -> None:
        """Start the Switch entity"""
        await self.async_subscribe(f"ucl/by-unid/{self._unid}/{self._ep}/OnOff/Attributes/OnOff/Reported", self._on_message_state)
        await self.async_subscribe(f"ucl/by-unid/{self._unid}/{self._ep}/OnOff/SupportedCommands", self._on_message_supported_commands)

    @final
    async def async_will_remove_from_hass(self):
        """Unsubscribe when removed."""
        for _unsubscribe_cb in self._async_mqtt_remove:
            _unsubscribe_cb()

    async def _on_message_supported_commands(self, message: ReceiveMessage):
        self.supported_commands = message.payload

    async def _on_message_state(self, message: ReceiveMessage):
        _LOGGER.debug("UnifyOnOff: state changed for %s to %s ",
                      self.name, message.topic)
        try:
            msg = json.loads(message.payload)
        except json.decoder.JSONDecodeError:
            return

        if msg["value"] is True:
            _LOGGER.debug("UnifyOnOff: state detected True ")
            self._is_on = True
        else:
            _LOGGER.debug("UnifyOnOff: state detected False ")
            self._is_on = False

        try:
            self.async_schedule_update_ha_state(False)
        except Exception as err:
            _LOGGER.error("UnifyOnOff: Exception on State Update: %s", err)

    @ property
    def unique_id(self):
        """Force update."""
        entity_unid = f"onoff_{self._unid}_{self._ep}"
        entity_unid = entity_unid.replace("-", "_")
        return entity_unid

    @ property
    def is_on(self):
        """Return true if sensor is on."""
        return self._is_on

    async def async_turn_on(self, **kwargs):
        if not "On" in self.supported_commands:
            _LOGGER.warning("UnifyOnOff: Unsupported ON command")
            return

        on_topic = f"ucl/by-unid/{self._unid}/{self._ep}/OnOff/Commands/On"
        on_payload = "{}"
        _LOGGER.debug("UnifyOnOff: TurnOn %s", on_topic)
        await self.async_send_message(on_topic, on_payload, False)

    async def async_turn_off(self, **kwargs):
        if not "Off" in self.supported_commands:
            _LOGGER.warning("UnifyOnOff: Unsupported OFF command")
            return

        off_topic = f"ucl/by-unid/{self._unid}/{self._ep}/OnOff/Commands/Off"
        off_payload = "{}"
        _LOGGER.debug("UnifyOnOff: TurnOff %s", off_topic)
        await self.async_send_message(off_topic, off_payload, False)

    async def async_send_message(self, topic, payload, retain):
        await mqtt.async_publish(self.hass, topic, payload, 0, retain)
