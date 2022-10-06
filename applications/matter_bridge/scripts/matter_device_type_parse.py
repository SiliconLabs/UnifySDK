# *****************************************************************************
# License
# <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
# ******************************************************************************
# The licensor of this software is Silicon Laboratories Inc. Your use of this
# software is governed by the terms of Silicon Labs Master Software License
# Agreement (MSLA) available at
# www.silabs.com/about-us/legal/master-software-license-agreement. This
# software is distributed to you in Source Code format and is governed by the
#  sections of the MSLA applicable to Source Code.
# *****************************************************************************

from ast import parse
import xml.etree.ElementTree as ET
import argparse
import json


def parse(input):
    with open(input) as xml_object:
        device_tree = ET.parse(xml_object)
        device_root = device_tree.getroot()
        return device_root


def create_map(device_root, map_string):
    for device in device_root:
        device_id = device.find("deviceId").text
        device_name = device.find("name").text
        clusters_supported = []
        for cluster in device.find("clusters"):
            if not cluster.attrib["server"] == "true":
                continue
            if not cluster.attrib["serverLocked"] == "true":
                continue
            if "cluster" in cluster.attrib.keys():
                clusters_supported.append(cluster.attrib["cluster"])
                continue

            clusters_supported.append(cluster.text)

        map_string += (
            "\t{{ {0}, {{{1}, {2}}} }}, \n".format(
                device_id, json.dumps(clusters_supported), json.dumps(device_name)
            )
            .replace("[", "{")
            .replace("]", "}")
        )

    return map_string[:-3]


parser = argparse.ArgumentParser(
    description="""
This script can generate the matter device types and their list of clusters.
it takes the CHIPDevice.xml file as input."""
)
parser.add_argument(
    "-input",
    help="input xml file",
    default="../../../components/uic_dotdot/dotdot-xml/CHIPDevice.xml",
)
parser.add_argument(
    "-output",
    help="Path to output files in. File will be named matter_device_types_clusters_list.inc",
    default="src/matter_device_types_clusters_list.inc",
)
args = parser.parse_args()
device_root = parse(args.input)

map_string = "#include <vector>\n"
map_string += "#include <map>\n"
map_string += "#include <string>\n"
map_string += "#include <app-common/zap-generated/ids/Clusters.h>\n"
map_string += "static const std::map<chip::DeviceTypeId, std::pair<std::vector<const char *>, const char *>> matter_device_type_vs_clusters_map = {\n"
map_string = create_map(device_root, map_string)
map_string += "\n"
map_string += "};"

with open(args.output, "w") as out:
    out.write(
        "// Following device type map is generated using matter_device_type_parse.py\n"
    )
    out.write(map_string)
