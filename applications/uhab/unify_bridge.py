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
import re
import json

# HA Libraries
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant

# Third party library
import paho.mqtt.client as mqtt

# Integration components
from custom_components.uhab.switch import UnifyOnOff

_LOGGER = logging.getLogger(__name__)


def find_endpoint(topic: str) -> str:
    found_endpoints = re.findall(r"/(ep\d+)/", topic)
    if len(found_endpoints) != 1:
        raise ValueError(f"More or less than 1 endpoints found in MQTT topic in device: {topic}")
    return found_endpoints[0]

def find_unid(topic: str) -> str:
    found_unids = re.findall(r"by-unid/([^/]*).*", topic)
    if len(found_unids) != 1:
        raise ValueError(f"More or less than 1 unids found in MQTT topic in device: {topic}")
    return found_unids[0]

def setup_device_data_structure(unid: str, endpoint: str, devices: dict):
    if unid not in devices:
        devices[unid] = {}
        devices[unid][endpoint] = {}
        return devices
    
    if endpoint not in devices[unid]:
        devices[unid][endpoint] = {}
        
    return devices

def setup_on_off_switch(unid: str, endpoint: str, hass: HomeAssistant, devices: dict, config: ConfigEntry, DOMAIN: str):
    config_base_topic = (
        f"ucl/by-unid/{unid}/{endpoint}/OnOff/Attributes/OnOff/Reported"
    )
    onoff = UnifyOnOff(hass, unid, endpoint, config_base_topic)
    
    devices[unid][endpoint]["onoff"] = onoff
    
    hass.helpers.discovery.load_platform(
        "switch", DOMAIN, {"unid": unid, "ep": endpoint}, config
    )

    _LOGGER.debug("MQTT: Added onoff cluster to device list")
    return devices, hass

class UnifyBridge:
    """Unify Bridge integrating Unify Framework into HA."""
    
    def __init__(self, hass: HomeAssistant, config: ConfigEntry, DOMAIN: str, CLIENT: str, DEVICES: str) -> None:
        self.client = mqtt.Client()
        self.hass = hass
        self.config = config
        self.DOMAIN = DOMAIN
        

        # Devies registered by the Unify HA Bridge.
        self.devices = {}
        
        # Hass setup
        self.hass.data[DOMAIN] = {}
        self.hass.data[DOMAIN][CLIENT] = self.client
        self.hass.data[DOMAIN][DEVICES] = self.devices

        # Clusters
        self.clusters_setup = {"OnOff": setup_on_off_switch}

    def start_bridge(self) -> bool:
        """Starts up the Unify MQTT bridge connecting devices to HA."""
        _LOGGER.info("Starting Unify Framework bridge")
        try:
            self.client.on_connect = self.on_connect
            self.client.on_message = self.on_message

            self.client.connect("localhost", 1883, 60)
            _LOGGER.info("MQTT: Connected")
            # Blocking call that processes network traffic, dispatches callbacks and
            # handles reconnecting.
            # Other loop*() functions are available that give a threaded interface and a
            # manual interface.
            self.client.loop_start()

        except Exception as err:
            _LOGGER.error("Unify MQTT Connection failed %s", err)
            return False
        
        return True
    
    def set_supported_commands(self, unid: str, endpoint: str, msg, entity: str):
        try:
            json_payload = json.loads(msg.payload)
        except json.decoder.JSONDecodeError:
            _LOGGER.warning(f"Supported commands could not be parsed: {msg.payload}")
            self.devices[unid][endpoint][entity].supported_commands = []
            return
        
        self.devices[unid][endpoint][entity].supported_commands = json_payload["value"]
    
    def on_message(self, client, userdata, msg):
        _LOGGER.debug("OnMessage Discovery")
        ucl_device_prefix = "ucl/by-unid"

        # Check if device exist
        if "State" in msg.topic:
            unid = find_unid(msg.topic)
            if unid in self.devices:
                for endpoint in self.devices[unid]:
                    for cluster in self.devices[unid][endpoint]:
                        self.devices[unid][endpoint][cluster].available = msg.payload
                        
        for cluster in self.clusters_setup:
            if mqtt.topic_matches_sub(f'{ucl_device_prefix}/+/+/{cluster}/#', msg.topic):
                _LOGGER.debug("Found device %s", msg.topic)
                unid = find_unid(msg.topic)
                endpoint = find_endpoint(msg.topic)
                _LOGGER.debug("MQTT: Device has UNID: %s", unid)
            
                self.devices = setup_device_data_structure(unid, endpoint, self.devices)
            
                if cluster.lower() not in self.devices[unid][endpoint]:
                    self.devices, self.hass = self.clusters_setup[cluster](unid, endpoint, self.hass, self.devices, self.config, self.DOMAIN)
                
                if "SupportedCommands" in msg.topic:
                    self.devices[unid][endpoint][cluster.lower()].supported_commands = msg.payload

    # The callback for when the client receives a CONNACK response from the server.
    def on_connect(self, client, userdata, flags, rc):
        _LOGGER.debug("MQTT: Connected with result code: %s", str(rc))
        self.subscribe_to_unify_devices()
    
    def subscribe_to_unify_devices(self):
        self.client.subscribe("ucl/by-unid/+/+/+/Attributes/+/Reported")
        self.client.subscribe("ucl/by-unid/+/+/+/SupportedCommands")
        self.client.subscribe("ucl/by-unid/+/State")
