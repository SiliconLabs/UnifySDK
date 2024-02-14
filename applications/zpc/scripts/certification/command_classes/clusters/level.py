import mqtt.mqtt_manager as mqtt_manager
import utils.utils as utils

CLUSTER_NAME = "Level"

def command_move_level(value, execute_if_off_mask=False, execute_if_off_override=False):
    mqtt_manager.send_unify_cluster_command(CLUSTER_NAME, "MoveToLevel",
                                           '{"Level":%d,"TransitionTime":0,"OptionsMask":{"ExecuteIfOff":%s,"CoupleColorTempToLevel":false},"OptionsOverride":{"ExecuteIfOff":%s,"CoupleColorTempToLevel":false} }'
                                           % (value, utils.bool_to_str(execute_if_off_mask), utils.bool_to_str(execute_if_off_override))
                                           )

def command_move_frequency(value):
    mqtt_manager.send_unify_cluster_command(CLUSTER_NAME, "MoveToClosestFrequency", '{"Frequency":%d}' % value)
