#!/usr/bin/python

# Copyright (c) 2014 Silicon Laboratories Inc.

from __future__ import print_function
import xml.dom.minidom
import sys
import os.path

gets = []
sets = []
reps = []
supported = []


if __name__ == '__main__':
    if(sys.argv[1] == '-'):
        x = xml.dom.minidom.parse(sys.stdin)
    else:
        x = xml.dom.minidom.parse(sys.argv[1])

    classes = dict()

    supported_dict = dict()

    for l in open(sys.argv[2]):
        t = l.split(";")
        if("support" in t[4]):
            supported_dict[t[2]] = True

    for c in x.getElementsByTagName("cmd_class"):
        cls = int(c.getAttribute("key"), 16)
        for q in c.childNodes:
            if(q.nodeType == 1 and q.tagName == "cmd"):
                cmd = int(q.getAttribute("key"), 16)
                name = q.getAttribute("name")
                if ("_GET" in q.getAttribute("name")):
                    gets.append("0x%2.2x%2.2x" % (cls, cmd))
                if ("_SET" in q.getAttribute("name")):
                    sets.append("0x%2.2x%2.2x" % (cls, cmd))
                if ("_REPORT" in q.getAttribute("name")):
                    reps.append("0x%2.2x%2.2x" % (cls, cmd))

                if(name in supported_dict):
                    supported.append("0x%2.2x%2.2x" % (cls, cmd))


gets = set(gets)
gets = list(gets)
gets.sort()

sets = set(sets)
sets = list(sets)
sets.sort()


reps = set(reps)
reps = list(reps)
reps.sort()

supported = set(supported)
supported = list(supported)
supported.sort()


print("// Generated with %s" % sys.argv[0])
print('#include "zwave_command_type.hpp"')
print("const std::set<uint16_t> ZwaveCommandClassType::get_list = {", end=' ')
i = 0
for g in gets:
    if(i & 0x7 == 0):
        print()
    print(g+",", end=' ')
    i = i+1
print("};")
print()
print("const std::set<uint16_t> ZwaveCommandClassType::set_list = {", end=' ')
i = 0
for g in sets:
    if(i & 0x7 == 0):
        print()
    print(g+",", end=' ')
    i = i+1
print("};")

print("const std::set<uint16_t> ZwaveCommandClassType::rep_list = {", end=' ')
i = 0
for g in reps:
    if(i & 0x7 == 0):
        print()
    print(g+",", end=' ')
    i = i+1
print("};")


print("const std::set<uint16_t> ZwaveCommandClassType::supported_list = {", end=' ')
i = 0
for g in supported:
    if(i & 0x7 == 0):
        print()
    print(g+",", end=' ')
    i = i+1
print("};")
