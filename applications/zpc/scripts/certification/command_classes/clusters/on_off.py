import mqtt.mqtt_manager as mqtt_manager

CLUSTER_NAME = "OnOff"

def command_on():
    mqtt_manager.send_unify_cluster_command(CLUSTER_NAME, "On")

def command_off():
    mqtt_manager.send_unify_cluster_command(CLUSTER_NAME, "Off")

def write_attributes_on_time(value):
    mqtt_manager.send_unify_cluster_command(CLUSTER_NAME, "WriteAttributes", '{"OnTime":%d}' % value)
