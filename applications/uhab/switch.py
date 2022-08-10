"""Unify binary switch on off"""

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

# Standard packages
import logging
import json

# HA packages
from homeassistant.components.switch import SwitchEntity
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity_platform import AddEntitiesCallback

# Integration components
from custom_components.uhab.base_unify_entity import BaseUnifyEntity


_LOGGER = logging.getLogger(__name__)

DOMAIN = "unify"
CLIENT = "mqtt_client"
DEVICES = "devices"

def setup_platform(
    hass: HomeAssistant, config, add_entities: AddEntitiesCallback, discovery_info=None
):
    if discovery_info is None:
        return
    
    device = hass.data[DOMAIN][DEVICES][discovery_info["unid"]][discovery_info["ep"]]
    _LOGGER.info("Setup Unify OnOff Platform: %s", device)
    
    if "onoff" in device:
        add_entities([device["onoff"]])
        return True
    
    _LOGGER.warning(f"Unable to add OnOff device: {discovery_info['device']}")
    return False

class UnifyOnOff(SwitchEntity, BaseUnifyEntity):
    """Representation of a Unify Switch."""

    def __init__(self, hass, unid, endpoint, state_topic):
        _LOGGER.debug("UnifyOnOff: Init")
        self._hass = hass
        self._state = False
        self._name = f"switch_{unid}_{endpoint}"
        self._unid = unid
        self._ep = endpoint
        self.client = hass.data[DOMAIN][CLIENT]
        self.state_topic = state_topic
        self.added_device = False

        try:
            self.client.message_callback_add(self.state_topic, self._on_message_state)
            _LOGGER.info("Created switch instance")
        except Exception as err:
            _LOGGER.error("UnifyOnOff: Failed initializing. Exception: %s", err)

    def _on_message_state(self, client, userdata, msg):
        _LOGGER.debug("UnifyOnOff: state changed for %s to %s ", self.name, msg.topic)
        try:
            msg = json.loads(msg.payload)
        except json.decoder.JSONDecodeError:
            return 
        
        if msg["value"] == True:
            _LOGGER.debug("UnifyOnOff: state detected True ")
            self._state = True
        else:
            _LOGGER.debug("UnifyOnOff: state detected False ")
            self._state = False
        
        try:
            if self.hass != None:
                self.schedule_update_ha_state(True)
        except Exception as err:
            _LOGGER.error("UnifyOnOff: Exception on State Update: %s", err)
            
    @property
    def unique_id(self):
        """Force update."""
        entity_unid = f"onoff_{self._unid}_{self._ep}"
        entity_unid = entity_unid.replace("-", "_")
        return entity_unid
    
    @property
    def name(self):
        """Return the name of the device."""
        return self._name

    @property
    def force_update(self):
        """Force update."""
        return True
    
    @property
    def is_on(self):
        """Return true if sensor is on."""
        return self._state

    @property
    def should_poll(self):
        """No polling needed."""
        return False

    def turn_on(self, **kwargs):
        if not "On" in self.supported_commands:
            _LOGGER.warning("UnifyOnOff: Unsupported ON command")
            return
        
        on_topic = f"ucl/by-unid/{self._unid}/{self._ep}/OnOff/Commands/On"
        on_payload = "{}"
        _LOGGER.debug("UnifyOnOff: TurnOn %s", on_topic)
        self.send_message(on_topic, on_payload, False)

    def turn_off(self, **kwargs):
        if not "Off" in self.supported_commands:
            _LOGGER.warning("UnifyOnOff: Unsupported OFF command")
            return
        
        off_topic = f"ucl/by-unid/{self._unid}/{self._ep}/OnOff/Commands/Off"
        off_payload = "{}"
        _LOGGER.debug("UnifyOnOff: TurnOff %s", off_topic)
        self.send_message(off_topic, off_payload, False)
    
    def send_message(self, topic, payload, retain):
        self.client.publish(topic, payload, 0, retain)