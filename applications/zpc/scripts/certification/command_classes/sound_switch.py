#!/usr/bin/env python3

import mqtt.mqtt_manager as mqtt_manager
import utils.pretty_print as display

from clusters import on_off, level

def change_default_tone(tone_id):
    display.action_description(f"Changing default tone ID to {tone_id}")
    level.command_move_frequency(tone_id)

def change_default_volume(volume):
    display.action_description(f"Changing default volume to {volume}")
    level.command_move_level(volume, True, True)

def play_tone(tone_id):
    display.action_description(f"Play tone {tone_id}")
    on_off.write_attributes_on_time(tone_id)

def play_default_tone():
    display.action_description(f"Play default tone")
    on_off.command_on()

def stop_tone():
    display.action_description(f"Stop tone")
    on_off.command_off()

if __name__ == '__main__':
    mqtt_manager.add_node()

    change_default_tone(2)
    change_default_volume(100)
    change_default_volume(0)
    play_tone(3)
    stop_tone()
    play_default_tone()

    mqtt_manager.remove_node()
