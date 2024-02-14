import configparser
import os
from pathlib import Path

FILE_NAME = "config.ini"

current_script_directory = Path(os.path.dirname(__file__))
config_directory = current_script_directory.resolve().parents[1]
config_file_path = config_directory / FILE_NAME

config = configparser.ConfigParser()
r = config.read(config_file_path)

if len(r) != 1:
    print(f"[ERROR] Can't find '{config_file_path}'. Exiting.")
    exit()


def get_zwave_config(): return config['ZWave']
def get_mqtt_config(): return config['MQTT']
