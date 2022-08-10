###############################################################################
## License
## <b>Copyright 2022  Silicon Laboratories Inc. www.silabs.com</b>
###############################################################################
## The licensor of this software is Silicon Laboratories Inc. Your use of this
## software is governed by the terms of Silicon Labs Master Software License
## Agreement (MSLA) available at
## www.silabs.com/about-us/legal/master-software-license-agreement. This
## software is distributed to you in Source Code format and is governed by the
## sections of the MSLA applicable to Source Code.
##
###############################################################################

# Standard library
import json
import logging
import asyncio
from enum import Enum

# HA components
from homeassistant.helpers.entity import Entity
from homeassistant.helpers.entity_registry import async_get_registry, EntityRegistry
from homeassistant.const import (
    STATE_OFF,
    STATE_ON,
    STATE_UNAVAILABLE,
    STATE_UNKNOWN,
)

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

        _LOGGER.debug(f"Failed parsing supported commands payload or it was empty: {commands_payload}")
        self._supported_commands = []
        
class BaseUnifyState(Entity):
    """Relay the state of the device to HA attribute availability"""
    
    _attr_available: bool = True
    _unify_state_ha_map = {"Online functional": STATE_ON, "Online interviewing": STATE_ON, "Offline": STATE_OFF, "Unavailable": STATE_UNAVAILABLE}
    _removing: bool = False
    
    @property
    def available(self):
        return self._attr_available
        
    @available.setter
    def available(self, state_payload: str):
        payload, parse_state = parse_payload(state_payload)
        if parse_state == ParseUnifyPayload.EMPTY:
            self._attr_available = False
            self.remove_entity()
            return
        
        if parse_state == ParseUnifyPayload.FAILED_PARSING:
            _LOGGER.debug(f"Failed to parse device state. Setting device to unavailable. State: {state_payload}")
            self._attr_available = False
            return
        
        self._unify_state = payload["NetworkStatus"]
        self._attr_available = payload["NetworkStatus"] == "Online functional"
        
        if self.hass != None:
            self.schedule_update_ha_state(force_refresh=True)
    
    def remove_entity(self):
        if self.hass != None and not self._removing:
            _LOGGER.debug(f"Removing device: {self.entity_id}")
            self._removing = True
            self.hass.add_job(self.async_remove(force_remove=True))
            hass_registry: EntityRegistry = asyncio.run_coroutine_threadsafe(async_get_registry(self.hass), self.hass.loop).result()
            hass_registry.async_remove(self.entity_id)
            self.schedule_update_ha_state(force_refresh=True)
    


class BaseUnifyEntity(BaseUnifySupportedCommands, BaseUnifyState):
    """Encapsulates all the base capabilities of a Unify device"""
    
        
