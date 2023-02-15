""" 
Copyright 2022 Silicon Laboratories, www.silabs.com

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""

"""The uhab integration."""
from __future__ import annotations
import logging
from .node_state_monitor import Node, NodeStateMonitor
from .const import DEVICES, DOMAIN, NODE_STATE_MONITOR

from homeassistant.config_entries import ConfigEntry
from homeassistant.const import Platform
from homeassistant.core import HomeAssistant
from homeassistant.helpers import discovery

_LOGGER = logging.getLogger(__name__)
PLATFORMS = [Platform.SWITCH, Platform.LIGHT, Platform.SENSOR]


async def async_setup_entry(hass: HomeAssistant, config: ConfigEntry) -> bool:
    """Set up uhab from a config entry."""
    hass.data[DOMAIN] = {}
    hass.data[DOMAIN][DEVICES] = {}

    async def async_on_state_changed(node: Node, old_state: str, new_state: str):
        if old_state != new_state:
            try:
                for endpoint in hass.data[DOMAIN][DEVICES][node.unid].value():
                    for entity in endpoint.values():
                        entity.async_schedule_update_ha_state(
                            force_refresh=True)
            except ValueError:
                _LOGGER.warning("Failed to set available")

    async def async_on_node_added(node):
        #_LOGGER.info(f"Node added: {node}")
        for endpoint in node.endpoints.values():
            discovery_info = {"unid": node.unid, "endpoint": endpoint.value}
            _LOGGER.info(
                f"Endpoint: {endpoint.value} clusters {endpoint.clusters.keys()}")
            if "ColorControl" in endpoint.clusters:
                hass.async_create_task(discovery.async_load_platform(
                    hass, Platform.LIGHT, DOMAIN, discovery_info, config))
                pass
            elif "Level" in endpoint.clusters and endpoint.clusters["Level"].supported_commands:
                hass.async_create_task(discovery.async_load_platform(
                    hass, Platform.LIGHT, DOMAIN, discovery_info, config))
                pass
            elif "OnOff" in endpoint.clusters:
                if endpoint.clusters["OnOff"].supported_generated_commands:
                    # TODO: Add binary sensor
                    pass
                if endpoint.clusters["OnOff"].supported_commands:
                    hass.async_create_task(discovery.async_load_platform(
                        hass, Platform.SWITCH, DOMAIN, discovery_info, config))
                    pass
            # sensors
            discovery_info["clusters"] = []
            if "Scenes" in endpoint.clusters:
                discovery_info["clusters"].append("scenes")
            if "TemperatureMeasurement" in endpoint.clusters:
                discovery_info["clusters"].append("temperaturemeasurement")
            if "PowerConfiguration" in endpoint.clusters:
                discovery_info["clusters"].append("powerconfiguration")
            if discovery_info["clusters"]:
                hass.async_create_task(discovery.async_load_platform(
                    hass, Platform.SENSOR, DOMAIN, discovery_info, config))

    hass.data[DOMAIN][NODE_STATE_MONITOR] = NodeStateMonitor(
        hass, async_on_node_added, async_on_state_changed)
    await hass.data[DOMAIN][NODE_STATE_MONITOR].async_start()
    return True


async def async_unload_entry(hass: HomeAssistant, entry: ConfigEntry) -> bool:
    """Unload a config entry."""
    if unload_ok := await hass.config_entries.async_unload_platforms(entry, PLATFORMS):
        hass.data[DOMAIN].pop(entry.entry_id)

    return unload_ok
