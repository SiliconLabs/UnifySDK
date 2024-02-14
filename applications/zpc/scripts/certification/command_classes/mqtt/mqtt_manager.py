import config.unify_config as config 
import utils.pretty_print as display 
import paho.mqtt.client as paho
import re

###########################
# Configuration objects
###########################
zwave_config = config.get_zwave_config()
mqtt_config = config.get_mqtt_config()


###########################
# ZWave config
###########################
# Network ID in Unify (from config file)
zwave_ctt_network_id = zwave_config['CTTControllerNetworkID']
# Full controller UUID 
zwave_ctt_controller_node = f"{zwave_ctt_network_id}-0001"
# add_node will fill this automatically, but you can fill it manually if you want
zwave_ctt_end_device_node = None


###########################
# MQTT logic
###########################

display.welcome_message(f"Welcome to the CTT Helper Python Script !")

# Create MQTT client
mqtt_client = paho.Client()
try:
    mqtt_client.connect(mqtt_config['host'], int(mqtt_config['port']), 420)
    display.check_success(f"Connected to MQTT {mqtt_config['host']}:{mqtt_config['port']}")
except Exception as e:
    display.check_fail(f"Could not connect to MQTT {mqtt_config['host']}:{mqtt_config['port']}")
    display.action_failed(e)
    exit()


# Subscribe to all topics. This will allow us to interpret any messages send by Unify
if mqtt_client.subscribe("ucl/#")[0] != 0:
    display.check_fail("Could not subscribe to \"ucl/#\"")
    exit()

print()


###########################
# Callbacks
###########################
# Check included devices and set its ID in zwave_ctt_end_device_node
def find_included_device(client, userdata, message):
    global zwave_ctt_end_device_node

    match = re.search("ucl/by-unid/(%s-([0-9A-F]{4}))/State" % zwave_ctt_network_id,  message.topic)
    # match 1 : all ID
    # match 2 : only ID ([0-9A-F]{4})
    if match and match.group(2) != "0001" and match.group(2) != "0002":
        zwave_ctt_end_device_node = match.group(1)

###########################
# Functions
###########################
# Make controller defined in config.ini in "Add node" mode
def add_node():
    global zwave_ctt_end_device_node

    display.action_description(f"Set {zwave_ctt_controller_node} in 'add node' mode")

    __mqtt_publish(f'ucl/by-unid/{zwave_ctt_controller_node}/ProtocolController/NetworkManagement/Write', '{"State":"add node"}')

    if zwave_ctt_end_device_node == None:
        mqtt_client.on_message = find_included_device
        
        display.action_step("Finding included node...")
        while zwave_ctt_end_device_node == None:
            mqtt_client.loop()
        display.action_success(f"Found included end device : {zwave_ctt_end_device_node}")
    else:
        display.action_warning(f"zwave_ctt_end_device_node already defined skipping automatic find of device : {zwave_ctt_end_device_node}")

    # Skip line to add readability
    print()

    return zwave_ctt_end_device_node

# Make controller defined in config.ini in "Remove node" mode
def remove_node():
    global zwave_ctt_end_device_node

    if zwave_ctt_end_device_node == None:
        display.action_failed("zwave_ctt_end_device_node is not defined. Call add_node before or set it manually.")
        return

    display.action_description(f"Set {zwave_ctt_controller_node} in 'remove node' mode")
    __mqtt_publish(f"ucl/by-unid/{zwave_ctt_controller_node}/ProtocolController/NetworkManagement/Write", '{"State":"remove node"}')

    # Undefined current device node
    zwave_ctt_end_device_node = None

# Send command to the endpoint added with add_node
def send_unify_cluster_command(cluster_name, cluster_command, mqtt_args = None, unify_node = None):
    """Send a command to the endpoint added by add_node() or the specified endpoint

    This function will publish the following topic : ucl/by-unid/{unify_node}/ep0/{cluster_name}/Commands/{cluster_command}

    Args:
        cluster_name (string): Unify cluster name (Level, OnOff, ...)
        cluster_command (string): Cluster command name
        mqtt_args (string, optional): Argument of the cluster command (e.g. '{"OnTime":%d}'). Defaults to None.
        unify_node (string, optional): Unify node ID (e.g zw-C108E238-0005). If you called add_node beforehand you should not have to specify it. Defaults to None.
    """

    unify_node = zwave_ctt_end_device_node if unify_node == None else unify_node

    if unify_node == None:
        display.action_failed("Can't get the endpoint ID. Either call add_node() to automatically discover the endpoint or manually specify it in command() function or setting zwave_ctt_end_device_node")
        return

    topic = f"ucl/by-unid/{unify_node}/ep0/{cluster_name}/Commands/{cluster_command}"

    __mqtt_publish(topic, mqtt_args)

    # Skip line to add readability
    print()

def __mqtt_publish(topic, args=None):
    args = args if args != None else "{}"
    display.publish_topic(f"'{topic} {args}'")

    info = mqtt_client.publish(topic, args)
    while not info.is_published():
        mqtt_client.loop()

    display.action_success(f"Command sent.")