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
import logging

# HA Libraries
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant

# Integration components
from custom_components.uhab.unify_bridge import UnifyBridge

DOMAIN = "unify"
CLIENT = "mqtt_client"
DEVICES = "devices"

_LOGGER = logging.getLogger(__name__)

def setup(hass: HomeAssistant, config: ConfigEntry):
    bridge = UnifyBridge(hass, config, DOMAIN, CLIENT, DEVICES)
    
    state = bridge.start_bridge()
    
    if not state:
        _LOGGER.error("Failed starting Unify Framework Bridge")
        return False
    
    return True