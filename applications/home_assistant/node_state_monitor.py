""" 
Copyright 2022 Silicon Laboratories, www.silabs.com

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""

"""This module monitors Nodes and their states from Unify"""
import json
import time
import re
import logging
import asyncio

from homeassistant.components import mqtt
from homeassistant.components.mqtt.models import ReceiveMessage

UCL_STATE_TOPIC = "ucl/by-unid/+/State"
UCL_NAME_LOCATION_TOPIC = "ucl/by-unid/+/+/NameAndLocation/Attributes/+/Reported"
UCL_SUPPORTED_COMMANDS_TOPIC = "ucl/by-unid/+/+/+/SupportedCommands"
UCL_SUPPORTED_GENERATED_COMMANDS_TOPIC = "ucl/by-unid/+/+/+/SupportedGeneratedCommands"
UCL_ATTRIBUTES_TOPIC = "ucl/by-unid/+/+/+/Attributes/+/Reported"

_LOGGER = logging.getLogger(__name__)


class Timer:
    def __init__(self, timeout, callback):
        self._timeout = timeout
        self._callback = callback
        self._task = asyncio.ensure_future(self._job())

    async def _job(self):
        await asyncio.sleep(self._timeout)
        await self._callback()

    def cancel(self):
        self._task.cancel()


class Node():
    """Unify node"""

    def __init__(self, unid):
        # _LOGGER.info(f"Created node: {unid}")
        node_added_timer = None
        self.unid = unid
        self.endpoints = {}
        self.last_update = time.time()
        self._state = "Offline"
        self.added = False

    @property
    def state(self):
        return self._state

    @state.setter
    def state(self, payload: str):
        try:
            self._state = json.loads(payload)["NetworkStatus"]
        except ValueError:
            pass


class Endpoint():
    """Unify Endpoint"""

    def __init__(self, value):
        self.clusters = {}
        self.value = value
        self._name = ""
        self._location = ""

    @property
    def name(self):
        return self._name

    @name.setter
    def name(self, payload: str):
        if payload == b'' or payload == '':
            self._name = ""
        else:
            try:
                self._name = json.loads(payload)["value"]
            except ValueError:
                pass

    @property
    def location(self):
        return self._location

    @location.setter
    def location(self, payload: str):
        if payload == b'' or payload == '':
            self._location = ""
        else:
            try:
                self._location = json.loads(payload)["value"]
            except ValueError:
                pass


class Cluster():
    """Unify Cluster"""

    def __init__(self, value):
        self.value = value
        self._supported_commands = []
        self._supported_generated_commands = []
        self.attributes = set()

    @property
    def supported_commands(self):
        """Get supported commands"""
        return self._supported_commands

    @supported_commands.setter
    def supported_commands(self, payload: str):
        try:
            self._supported_commands = json.loads(payload)["value"]
        except ValueError:
            pass

    @property
    def supported_generated_commands(self):
        """Get supported generated commands"""
        return self._supported_generated_commands

    @supported_generated_commands.setter
    def supported_generated_commands(self, payload: str):
        try:
            self._supported_generated_commands = json.loads(payload)["value"]
        except ValueError:
            pass


class NodeStateMonitor():
    """Monitor Unify nodes"""

    def __init__(self, hass, async_on_node_added, async_on_state_changed) -> None:
        _LOGGER.info("Created NodeStateMonitor")
        self._hass = hass
        self._unsubscribe_callback = []
        self.nodes = {}
        self._async_on_node_added = async_on_node_added
        self._async_on_state_changed = async_on_state_changed

    async def async_start(self) -> None:
        """Start the node state monitor"""
        _LOGGER.info("Starting NodeStateMonitor")
        self._unsubscribe_callback.append(await mqtt.async_subscribe(self._hass, UCL_SUPPORTED_COMMANDS_TOPIC, self._on_supported_command_message))
        self._unsubscribe_callback.append(await mqtt.async_subscribe(self._hass, UCL_SUPPORTED_GENERATED_COMMANDS_TOPIC, self._on_supported_generated_command_message))
        self._unsubscribe_callback.append(await mqtt.async_subscribe(self._hass, UCL_ATTRIBUTES_TOPIC, self._on_attribute_reported_message))
        self._unsubscribe_callback.append(await mqtt.async_subscribe(self._hass, UCL_NAME_LOCATION_TOPIC, self._on_node_name_or_location_reported_message))
        self._unsubscribe_callback.append(await mqtt.async_subscribe(self._hass, UCL_STATE_TOPIC, self._on_node_state_message))

    async def async_stop(self) -> None:
        """Stop the node state monitor"""
        _LOGGER.info("Stopping NodeStateMonitor")
        for unsubscribe_cb in self._unsubscribe_callback:
            unsubscribe_cb()

    async def _on_supported_command_message(self, message: ReceiveMessage):
        try:
            _unid, _endpoint, _cluster = re.findall(("ucl/by-unid"
                                                     "/([^/]*)"  # UNID
                                                     "/([^/]*)"  # Endpoint
                                                     "/([^/]*)"  # Cluster
                                                     "/SupportedCommands")  # SupportedCommands
                                                    , message.topic)[0]
            node: Node = self.nodes.setdefault(_unid, Node(_unid))
            node.last_update = time.time()
            node.endpoints.setdefault(_endpoint, Endpoint(_endpoint))   \
                .clusters.setdefault(_cluster, Cluster(_cluster))       \
                .supported_commands = message.payload

        except ValueError as err:
            _LOGGER.warning(f"Failed parsing topic in _on_supported_command_message: {err}")

    async def _on_supported_generated_command_message(self, message: ReceiveMessage):
        try:
            _unid, _endpoint, _cluster = re.findall(("ucl/by-unid"
                                                     "/([^/]*)"  # UNID
                                                     "/([^/]*)"  # Endpoint
                                                     "/([^/]*)"  # Cluster
                                                     "/SupportedGeneratedCommands")  # SupportedCommands
                                                    , message.topic)[0]

            node: Node = self.nodes.setdefault(_unid, Node(_unid))
            node.last_update = time.time()
            node.endpoints.setdefault(_endpoint, Endpoint(_endpoint))   \
                .clusters.setdefault(_cluster, Cluster(_cluster))       \
                .supported_generated_commands = message.payload

        except ValueError as err:
            _LOGGER.warning(f"Failed parsing topic in _on_supported_generated_command_message: {err}")

    async def _on_attribute_reported_message(self, message: ReceiveMessage):
        try:
            _unid, _endpoint, _cluster, _attribute = re.findall(("ucl/by-unid"
                                                                 "/([^/]*)"       # UNID
                                                                 "/([^/]*)"       # Endpoint
                                                                 "/([^/]*)"       # Cluster
                                                                 "/Attributes"    # Attributes
                                                                 "/([^/]*)"        # Attribute
                                                                 "/Reported")
                                                                , message.topic)[0]
            node: Node = self.nodes.setdefault(_unid, Node(_unid))
            cluster: Cluster = node.endpoints.setdefault(_endpoint, Endpoint(_endpoint))   \
                .clusters.setdefault(_cluster, Cluster(_cluster))
            if (message.payload == b'' or message.payload == '') and _attribute in cluster.attributes:
                cluster.attributes.remove(_attribute)
                node.last_update = time.time()
            else:
                if _attribute not in cluster.attributes:
                    cluster.attributes.add(_attribute)
                    node.last_update = time.time()

        except ValueError as err:
            _LOGGER.warning(f"Failed parsing topic in _on_attribute_reported_message: {err}")

    async def _on_node_state_message(self, message: ReceiveMessage):
        _LOGGER.info(f"on_state_message {message.topic}, {message.payload}")
        _unid = re.findall("ucl/by-unid/([^/]*)/State", message.topic)[0]
        if message.payload == b'' or message.payload == '':
            self.nodes.pop(_unid, None)
        node: Node = self.nodes.setdefault(_unid, Node(_unid))
        old_state = node.state
        node.state = message.payload

        if not node.added and self._async_on_node_added and not hasattr(node, 'node_added_timer'):
            async def timeout_callback():
                if time.time() - node.last_update > 1:
                    node.node_added_timer = None
                    _LOGGER.info("Executing on_node_added")
                    node.added = True
                    await self._async_on_node_added(node)
                else:
                    _LOGGER.info("Starting node_added_timer")
                    node.node_added_timer = Timer(1, timeout_callback)
            await timeout_callback()
        else:
            if self._async_on_state_changed:
                await self._async_on_state_changed(node, old_state, node.state)

    async def _on_node_name_or_location_reported_message(self, message: ReceiveMessage):
        _LOGGER.info(f"on_state_message {message.topic}, {message.payload}")
        _unid, _endpoint , _attribute = re.findall(("ucl/by-unid"
                                                    "/([^/]*)"          # UNID
                                                    "/([^/]*)"          # Endpoint
                                                    "/NameAndLocation"  # Cluster
                                                    "/Attributes"       # Attributes
                                                    "/([^/]*)"          # Attribute
                                                    "/Reported"), message.topic)[0]
        node: Node = self.nodes.setdefault(_unid, Node(_unid))
        endpoint = node.endpoints.setdefault(_endpoint, Endpoint(_endpoint))

        if "Name" == _attribute:
            endpoint.name = message.payload
        elif "Location" == _attribute:
            endpoint.location = message.payload
