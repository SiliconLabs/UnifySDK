""" 
Copyright 2022 Silicon Laboratories, www.silabs.com

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""

# Standard library
import json
import logging
import asyncio
import re
from enum import Enum
from homeassistant.core import HomeAssistant, callback

# HA components
from homeassistant.helpers.entity import Entity
from homeassistant.helpers.entity_registry import async_get_registry, EntityRegistry
from homeassistant.const import (
    STATE_OFF,
    STATE_ON,
    STATE_UNAVAILABLE)
from homeassistant.components import mqtt

# From this component
from .node_state_monitor import Endpoint, Node
from .const import DOMAIN, DEVICES, NODE_STATE_MONITOR

_LOGGER = logging.getLogger(__name__)


class ParseUnifyPayload(Enum):
    SUCCESS = 1
    FAILED_PARSING = 2
    EMPTY = 3


def parse_payload(payload: str):
    if payload == b'' or payload == '':
        return {}, ParseUnifyPayload.EMPTY

    try:
        json_payload = json.loads(payload)
    except json.decoder.JSONDecodeError:
        _LOGGER.warning(f"Failed parsing payload: {payload}")
        return {}, ParseUnifyPayload.FAILED_PARSING

    return json_payload, ParseUnifyPayload.SUCCESS


def find_endpoint(topic: str) -> str:
    found_endpoints = re.findall(r"/(ep\d+)/", topic)
    if len(found_endpoints) != 1:
        raise ValueError(
            f"More or less than 1 endpoints found in MQTT topic in device: {topic}")
    return found_endpoints[0]


def find_unid(topic: str) -> str:
    found_unids = re.findall(r"by-unid/([^/]*).*", topic)
    if len(found_unids) != 1:
        raise ValueError(
            f"More or less than 1 unids found in MQTT topic in device: {topic}")
    return found_unids[0]


def setup_device_data_structure(unid: str, endpoint: str, devices: dict):
    if unid not in devices:
        devices[unid] = {}
        devices[unid][endpoint] = {}
        return devices

    if endpoint not in devices[unid]:
        devices[unid][endpoint] = {}

    return devices


def get_hass_endpoint(hass: HomeAssistant, node: Node, endpoint: Endpoint):
    if node.unid not in hass.data[DOMAIN][DEVICES]:
        hass.data[DOMAIN][DEVICES][node.unid] = {}
    if endpoint not in hass.data[DOMAIN][DEVICES][node.unid]:
        hass.data[DOMAIN][DEVICES][node.unid][endpoint.name] = {}
    return hass.data[DOMAIN][DEVICES][node.unid][endpoint.name]


class BaseUnifySupportedCommands:
    """The base entity implements standard functionality for Unify UCL
    attributes. The base entity is generic for all device types in Unify UCL."""

    _supported_commands = []

    @property
    def supported_commands(self):
        return self._supported_commands

    @supported_commands.setter
    def supported_commands(self, commands_payload: str):
        payload, parse_state = parse_payload(commands_payload)

        if parse_state == ParseUnifyPayload.SUCCESS:
            self._supported_commands = payload["value"]
            return

        _LOGGER.debug(
            f"Failed parsing supported commands payload or it was empty: {commands_payload}")
        self._supported_commands = []


class BaseUnifyState(Entity):
    """Relay the state of the device to HA attribute availability"""

    _attr_available: bool = True
    _unify_state_ha_map = {"Online functional": STATE_ON, "Online interviewing": STATE_ON,
                           "Offline": STATE_OFF, "Unavailable": STATE_UNAVAILABLE}
    _removing: bool = False

    @property
    def available(self):
        try:
            state = self.hass.data[DOMAIN][NODE_STATE_MONITOR].nodes[self._unid].state
            return state == "Online functional"
        except:
            return False

    @callback
    async def async_remove_entity(self):
        if self.hass is not None and not self._removing:
            _LOGGER.debug(f"Removing device: {self.entity_id}")
            self._removing = True
            self.hass.add_job(self.async_remove(force_remove=True))
            hass_registry: EntityRegistry = asyncio.run_coroutine_threadsafe(
                async_get_registry(self.hass), self.hass.loop).result()
            hass_registry.async_remove(self.entity_id)
            await self.async_schedule_update_ha_state(force_refresh=True)


class BaseUnifyEntity(BaseUnifySupportedCommands, BaseUnifyState):
    """Encapsulates all the base capabilities of a Unify device"""

    def __init__(self, hass, unid, endpoint):
        self._hass = hass
        self._unid = unid
        self._ep = endpoint
        self.added_device = False
        self._async_mqtt_remove = []
        self._name = hass.data[DOMAIN][NODE_STATE_MONITOR].nodes[self._unid].endpoints[self._ep].name
        self._location = hass.data[DOMAIN][NODE_STATE_MONITOR].nodes[self._unid].endpoints[self._ep].location

    @ property
    def name(self):
        """Return the name of the device."""
        return self._name or self.unique_id

    @ property
    def force_update(self):
        """Force update."""
        return True

    @ property
    def should_poll(self):
        """No polling needed."""
        return False

    async def async_subscribe(self, topic, _callback):
        """Subscribe to an MQTT topic"""
        self._async_mqtt_remove.append(await mqtt.async_subscribe(self._hass, topic, _callback))
