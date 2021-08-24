import csv
import argparse

# This script takes in the Z-Wave Notification Command Class list of assigned
# notifications and outputs C++ defines and map<notification, <event, state>>
# table.
# It generates ID's for the states under each notification


def to_c_define(_str):
    return ''.join(e for e in _str.replace(' ', '_') if (e.isalnum() or e == "_")).upper()


def parse(input, output):
    with open(input) as csvfile:
        notreader = csv.reader(csvfile, delimiter=';')
        table = dict()
        _state = None
        _id = None
        for row in notreader:
            if row[0]:
                try:
                    _id = int(row[1], 16)
                    _state = row[0]
                    table[_state] = {'state': _state, 'id': _id,
                                     'events': dict(), 'states': dict()}
                except:
                    #print("Skipping row: " + ', '.join(row))
                    pass
            if _state:
                _type = row[2]
                _variable = row[3]
                _event = int(row[6], 16)
                if _event == 0:
                    continue
                if "State" in _type:
                    if not _variable in table[_state]['states'].keys():
                        table[_state]['states'][_variable] = []
                    table[_state]['states'][_variable].append(_event)

        # for entry in table:
        #    print(table[entry])

        defines_state_str = "#define NOTIFICATION_STATE_LAST_EVENT (0xff)\n"
        def_state_str = "def NOTIFICATION_STATE_LAST_EVENT (0xff)\n"

        defines_notifications_str = ""
        def_notifications_str = ""

        rules_str = ""
        test_str = ""

        table_str = "// map<notification_type, map<event_state_value, state>>\n"
        table_str += "static const std::map<unsigned int, std::map<unsigned int, unsigned int>> notification_event_state_map = {"
        for entry in table:
            if len(table[entry]['states']) == 0:
                continue
            # { notification, {
            notification_str = to_c_define('NOTIFICATION_' + entry)
            defines_notifications_str += "#define {} ({})\n".format(
                notification_str, hex(table[entry]['id']))

            def_notifications_str += "def {} {}\n".format(
                notification_str, hex(table[entry]['id']))
            
            table_str += '{' + notification_str + ', '
            state_counter = 0

            table_str += '{'

            rules_str += "\n// {}\n".format(notification_str)
            test_str += "\n// {}\n".format(notification_str)

            for state in table[entry]['states']:
                # Create C style define for states <notification>_<state> in uppercase with _ for spaces
                state_str = 'NOTIFICATION_STATE_' + ''.join(
                    e for e in entry + ' ' + state if (e.isalnum() or e == " ")).replace(' ', '_').upper()
                defines_state_str += "#define {} ({})\n".format(
                    state_str, hex(state_counter))

                def_state_str += "def {} {}\n".format(
                    state_str, hex(state_counter))

                rules_str +=  "def {} (r'zwNOTIFICATION_TYPE[{}].zwNOTIFICATION_STATE[{}].zwNOTIFICATION_EVENT)\n".format(
                     state_str[len('NOTIFICATION_STATE_'):], to_c_define('NOTIFICATION_' + entry), state_str)    

                test_str += "create_branch(\"ATTRIBUTE_NOTIFICATION_NOTIFICATION_TYPE[{}].ATTRIBUTE_NOTIFICATION_NOTIFICATION_STATE[{}].ATTRIBUTE_NOTIFICATION_NOTIFICATION_EVENT\")\n".format(
                    hex(table[entry]['id']), hex(state_counter))             
                #table_str += '{' + str(state_counter) + ', {'
                for event_val in table[entry]['states'][state]:
                    # {event, state},
                    table_str += '{' + str(hex(event_val)) + \
                        ', ' + state_str + '}, '
                table_str = table_str[:-2]
                # }
                state_counter += 1
                table_str += ', '
            table_str = table_str[:-2]
            # }
            table_str += '}},'
            table_str += "\n"
        table_str = table_str[:-2]
        table_str += "};"
        with open(output + 'zwave_command_class_notification_types.h', 'w') as out:
            out.write('''/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

/**
 * @file zwave_command_class_notification_types.h
 * @brief Notification Types and states definitions
 *
 * The definitions are generated by the notification.py script
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_NOTIFICATION_TYPES_H
#define ZWAVE_COMMAND_CLASS_NOTIFICATION_TYPES_H

''')
            out.write("// Notification Types\n")
            out.write(defines_notifications_str)
            out.write("\n// Notification States\n")
            out.write(defines_state_str)
            out.write('''
#endif  //ZWAVE_COMMAND_CLASS_NOTIFICATION_TYPES_H
/** @} end zwave_command_class_notification_types */
''')
        with open(output + 'zwave_command_class_notification_helpers.txt', 'w') as out:
            out.write("// Following helpers are generated using notification.py script\n")
            out.write("// Helpers for Notification Command Class mappings\n")
            out.write("\n// Notification Types\n")
            out.write(def_notifications_str)
            out.write("\n// Notification States\n")
            out.write(def_state_str)
            out.write("\n// Notification CC Rules Helpers\n")
            out.write(rules_str)
            out.write("\n// Notification CC Test Helpers\n")
            out.write(test_str)


        with open(output + 'zwave_command_class_notification_types.inc', 'w') as out:
            out.write(
                "// Following map is generated using notification.py script\n")
            out.write(table_str)
        # print(defines_notifications_str)
        # print(defines_state_str)
        # print(table_str)


parser = argparse.ArgumentParser(description='''
This script can generate notification command class types and lookup tables for
states for the notification command class. 
It takes a csv file as input, this file can be created by opening the 
"Notification Command Class, list of assigned Notifications <version>.xls"
file and exporting the "Notifications" sheet as csv.''')
parser.add_argument('-input', help='Input csv file',
                    default='../assets/notification.csv')
parser.add_argument(
    '-output', help='Path to store output files in. Files will be named zwave_command_class_notification_types.inc/h/txt', default='../src/')
args = parser.parse_args()
parse(args.input, args.output)
