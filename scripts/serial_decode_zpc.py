#!/usr/bin/python
# SPDX-License-Identifier: LicenseRef-MSLA
# SPDX-FileCopyrightText: Silicon Laboratories Inc. https://www.silabs.com
'''
The script was replicated and updated to work with Z/IP Gateway
See https://github.com/SiliconLabs/zipgateway/pull/30

Created on 08/11/2011

@author: aes
'''
import sys
    
SOF=0x01
ACK=0x06
CAN=0x18
NAK=0x15

funcids=dict()

funcids[0x01] = "FUNC_ID_SERIAL_API_HELO"
funcids[0x02] = "FUNC_ID_SERIAL_API_GET_INIT_DATA"
funcids[0x03] = "FUNC_ID_SERIAL_API_APPL_NODE_INFORMATION"
funcids[0x04] = "FUNC_ID_APPLICATION_COMMAND_HANDLER"
funcids[0x05] = "FUNC_ID_ZW_GET_CONTROLLER_CAPABILITIES"
funcids[0x06] = "FUNC_ID_SERIAL_API_SET_TIMEOUTS"
funcids[0x07] = "FUNC_ID_SERIAL_API_GET_CAPABILITIES"
funcids[0x08] = "FUNC_ID_SERIAL_API_SOFT_RESET"
funcids[0x09] = "FUNC_ID_ZW_GET_PROTOCOL_VERSION"
funcids[0x10] = "FUNC_ID_ZW_SET_RF_RECEIVE_MODE"
funcids[0x0a] = "FUNC_ID_SERIALAPI_STARTED"
funcids[0x0b] = "FUNC_ID_SERIALAPI_SETUP"
funcids[0x11] = "FUNC_ID_ZW_SET_SLEEP_MODE"
funcids[0x12] = "FUNC_ID_ZW_SEND_NODE_INFORMATION"
funcids[0x13] = "FUNC_ID_ZW_SEND_DATA"
funcids[0x14] = "FUNC_ID_ZW_SEND_DATA_MULTI"
funcids[0x15] = "FUNC_ID_ZW_GET_VERSION"
funcids[0x16] = "FUNC_ID_ZW_SEND_DATA_ABORT"
funcids[0x17] = "FUNC_ID_ZW_RF_POWER_LEVEL_SET"
funcids[0x18] = "FUNC_ID_ZW_SEND_DATA_META"
funcids[0x19] = "FUNC_ID_ZW_RESERVED_SD"
funcids[0x1A] = "FUNC_ID_ZW_RESERVED_SDM"
funcids[0x1B] = "FUNC_ID_ZW_RESERVED_SRI"
funcids[0x1B] = "FUNC_ID_ZW_SET_ROUTING_INFO"
funcids[0x1C] = "FUNC_ID_ZW_GET_RANDOM"
funcids[0x1D] = "FUNC_ID_ZW_RANDOM"
funcids[0x1E] = "FUNC_ID_ZW_RF_POWER_LEVEL_REDISCOVERY_SET"
funcids[0x20] = "FUNC_ID_MEMORY_GET_ID"
funcids[0x21] = "FUNC_ID_MEMORY_GET_BYTE"
funcids[0x22] = "FUNC_ID_MEMORY_PUT_BYTE"
funcids[0x23] = "FUNC_ID_MEMORY_GET_BUFFER"
funcids[0x24] = "FUNC_ID_MEMORY_PUT_BUFFER"
funcids[0x25] = "FUNC_ID_SERIAL_API_GET_APPL_HOST_MEMORY_OFFSET"
funcids[0x26] = "FUNC_ID_DEBUG_OUTPUT"
funcids[0x27] = "FUNC_ID_AUTO_PROGRAMMING"
funcids[0x28] = "FUNC_ID_NVR_GET_VALUE"
funcids[0x29] = "FUNC_ID_NVM_GET_ID"
funcids[0x2A] = "FUNC_ID_NVM_EXT_READ_LONG_BUFFER"
funcids[0x2B] = "FUNC_ID_NVM_EXT_WRITE_LONG_BUFFER"
funcids[0x2C] = "FUNC_ID_NVM_EXT_READ_LONG_BYTE"
funcids[0x2D] = "FUNC_ID_NVM_EXT_WRITE_LONG_BYTE"
funcids[0x2E] = "FUNC_ID_NVM_BACKUP_RESTORE"
funcids[0x30] = "FUNC_ID_CLOCK_SET"
funcids[0x31] = "FUNC_ID_CLOCK_GET"
funcids[0x32] = "FUNC_ID_CLOCK_CMP"
funcids[0x33] = "FUNC_ID_RTC_TIMER_CREATE"
funcids[0x34] = "FUNC_ID_RTC_TIMER_READ"
funcids[0x35] = "FUNC_ID_RTC_TIMER_DELETE"
funcids[0x36] = "FUNC_ID_RTC_TIMER_CALL"
funcids[0x37] = "FUNC_ID_CLEAR_TX_TIMERS"
funcids[0x38] = "FUNC_ID_GET_TX_TIMERS"
funcids[0x3B] = "FUNC_ID_ZW_GET_BACKGROUND_RSSI"
funcids[0x3F] = "FUNC_ID_ZW_REMOVE_NODE_ID_FROM_NETWORK"
funcids[0x40] = "FUNC_ID_ZW_SET_LEARN_NODE_STATE"
funcids[0x41] = "FUNC_ID_ZW_GET_NODE_PROTOCOL_INFO"
funcids[0x42] = "FUNC_ID_ZW_SET_DEFAULT"
funcids[0x43] = "FUNC_ID_ZW_NEW_CONTROLLER"
funcids[0x44] = "FUNC_ID_ZW_REPLICATION_COMMAND_COMPLETE"
funcids[0x45] = "FUNC_ID_ZW_REPLICATION_SEND_DATA"
funcids[0x46] = "FUNC_ID_ZW_ASSIGN_RETURN_ROUTE"
funcids[0x47] = "FUNC_ID_ZW_DELETE_RETURN_ROUTE"
funcids[0x48] = "FUNC_ID_ZW_REQUEST_NODE_NEIGHBOR_UPDATE"
funcids[0x49] = "FUNC_ID_ZW_APPLICATION_UPDATE"
funcids[0x49] = "FUNC_ID_ZW_APPLICATION_CONTROLLER_UPDATE"
funcids[0x4A] = "FUNC_ID_ZW_ADD_NODE_TO_NETWORK"
funcids[0x4B] = "FUNC_ID_ZW_REMOVE_NODE_FROM_NETWORK"
funcids[0x4C] = "FUNC_ID_ZW_CREATE_NEW_PRIMARY"
funcids[0x4D] = "FUNC_ID_ZW_CONTROLLER_CHANGE"
funcids[0x4E] = "FUNC_ID_ZW_RESERVED_FN"
funcids[0x4F] = "FUNC_ID_ZW_RESERVED_AR"
funcids[0x50] = "FUNC_ID_ZW_SET_LEARN_MODE"
funcids[0x51] = "FUNC_ID_ZW_ASSIGN_SUC_RETURN_ROUTE"
funcids[0x52] = "FUNC_ID_ZW_ENABLE_SUC"
funcids[0x53] = "FUNC_ID_ZW_REQUEST_NETWORK_UPDATE"
funcids[0x54] = "FUNC_ID_ZW_SET_SUC_NODE_ID"
funcids[0x55] = "FUNC_ID_ZW_DELETE_SUC_RETURN_ROUTE"
funcids[0x56] = "FUNC_ID_ZW_GET_SUC_NODE_ID"
funcids[0x57] = "FUNC_ID_ZW_SEND_SUC_ID"
funcids[0x58] = "FUNC_ID_ZW_RESERVED_ASR"
funcids[0x59] = "FUNC_ID_ZW_REDISCOVERY_NEEDED"
funcids[0x5A] = "FUNC_ID_ZW_REQUEST_NODE_NEIGHBOR_UPDATE_OPTION"
funcids[0x5B] = "FUNC_ID_ZW_SUPPORT9600_ONLY"
funcids[0x5C] = "FUNC_ID_ZW_REQUEST_NEW_ROUTE_DESTINATIONS"
funcids[0x5D] = "FUNC_ID_ZW_IS_NODE_WITHIN_DIRECT_RANGE"
funcids[0x5E] = "FUNC_ID_ZW_EXPLORE_REQUEST_INCLUSION"
funcids[0x5F] = "FUNC_ID_ZW_EXPLORE_REQUEST_EXCLUSION"
funcids[0x60] = "FUNC_ID_ZW_REQUEST_NODE_INFO"
funcids[0x61] = "FUNC_ID_ZW_REMOVE_FAILED_NODE_ID"
funcids[0x62] = "FUNC_ID_ZW_IS_FAILED_NODE_ID"
funcids[0x63] = "FUNC_ID_ZW_REPLACE_FAILED_NODE"
funcids[0x65] = "FUNC_ID_ZW_SET_ROUTING_MAX_6_00"
funcids[0x66] = "FUNC_ID_ZW_IS_PRIMARY_CTRL"
funcids[0x67] = "FUNC_ID_ZW_AES_ECB"
funcids[0x70] = "FUNC_ID_TIMER_START"
funcids[0x71] = "FUNC_ID_TIMER_RESTART"
funcids[0x72] = "FUNC_ID_TIMER_CANCEL"
funcids[0x73] = "FUNC_ID_TIMER_CALL"
funcids[0x80] = "FUNC_ID_GET_ROUTING_TABLE_LINE"
funcids[0x81] = "FUNC_ID_GET_TX_COUNTER"
funcids[0x82] = "FUNC_ID_RESET_TX_COUNTER"
funcids[0x83] = "FUNC_ID_STORE_NODEINFO"
funcids[0x84] = "FUNC_ID_STORE_HOMEID"
funcids[0x90] = "FUNC_ID_LOCK_ROUTE_RESPONSE"
funcids[0x91] = "FUNC_ID_ZW_SEND_DATA_ROUTE_DEMO"
funcids[0x92] = "FUNC_ID_ZW_GET_LAST_WORKING_ROUTE"
funcids[0x93] = "FUNC_ID_ZW_SET_LAST_WORKING_ROUTE"
funcids[0x95] = "FUNC_ID_SERIAL_API_TEST"
funcids[0x98] = "FUNC_ID_SERIAL_API_EXT"
funcids[0xA0] = "FUNC_ID_SERIAL_API_APPL_SLAVE_NODE_INFORMATION"
funcids[0xA1] = "FUNC_ID_APPLICATION_SLAVE_COMMAND_HANDLER"
funcids[0xA2] = "FUNC_ID_ZW_SEND_SLAVE_NODE_INFORMATION"
funcids[0xA3] = "FUNC_ID_ZW_SEND_SLAVE_DATA"
funcids[0xA4] = "FUNC_ID_ZW_SET_SLAVE_LEARN_MODE"
funcids[0xA5] = "FUNC_ID_ZW_GET_VIRTUAL_NODES"
funcids[0xA6] = "FUNC_ID_ZW_IS_VIRTUAL_NODE"
funcids[0xA7] = "FUNC_ID_ZW_RESERVED_SSD"
funcids[0xA8] = "FUNC_ID_APPLICATION_COMMAND_HANDLER_BRIDGE"
funcids[0xA9] = "FUNC_ID_ZW_SEND_DATA_BRIDGE"
funcids[0xAA] = "FUNC_ID_ZW_SEND_DATA_META_BRIDGE"
funcids[0xAB] = "FUNC_ID_ZW_SEND_DATA_MULTI_BRIDGE"
funcids[0xB0] = "FUNC_ID_PWR_SETSTOPMODE"
funcids[0xB1] = "FUNC_ID_PWR_CLK_PD"
funcids[0xB2] = "FUNC_ID_PWR_CLK_PUP"
funcids[0xB3] = "FUNC_ID_PWR_SELECT_CLK"
funcids[0xB4] = "FUNC_ID_ZW_SET_WUT_TIMEOUT"
funcids[0xB5] = "FUNC_ID_ZW_IS_WUT_KICKED"
funcids[0xB6] = "FUNC_ID_ZW_WATCHDOG_ENABLE"
funcids[0xB7] = "FUNC_ID_ZW_WATCHDOG_DISABLE"
funcids[0xB8] = "FUNC_ID_ZW_WATCHDOG_KICK"
funcids[0xB9] = "FUNC_ID_ZW_SET_EXT_INT_LEVEL"
funcids[0xB9] = "FUNC_ID_ZW_INT_EXT_LEVEL_SET"
funcids[0xBA] = "FUNC_ID_ZW_RF_POWER_LEVEL_GET"
funcids[0xBB] = "FUNC_ID_ZW_GET_NEIGHBOR_COUNT"
funcids[0xBC] = "FUNC_ID_ZW_ARE_NODES_NEIGHBOURS"
funcids[0xBD] = "FUNC_ID_ZW_TYPE_LIBRARY"
funcids[0xBE] = "FUNC_ID_ZW_SEND_TEST_FRAME"
funcids[0xBF] = "FUNC_ID_ZW_GET_PROTOCOL_STATUS"
funcids[0xD0] = "FUNC_ID_ZW_SET_PROMISCUOUS_MODE"
funcids[0xD1] = "FUNC_ID_PROMISCUOUS_APPLICATION_COMMAND_HANDLER"
funcids[0xD2] = "FUNC_ID_ZW_WATCHDOG_START"
funcids[0xD3] = "FUNC_ID_ZW_WATCHDOG_STOP"
funcids[0xD4] = "FUNC_ID_ZW_SET_ROUTING_MAX"
funcids[0xD5] = "FUNC_ID_ZW_GET_ROUTING_MAX"
funcids[0xE0] = "FUNC_ID_ZW_NUNIT_CMD"
funcids[0xE1] = "FUNC_ID_ZW_NUNIT_INIT"
funcids[0xE2] = "FUNC_ID_ZW_NUNIT_LIST"
funcids[0xE3] = "FUNC_ID_ZW_NUNIT_RUN"
funcids[0xE4] = "FUNC_ID_ZW_NUNIT_END"
funcids[0xE5] = "FUNC_ID_IO_PORT_STATUS"
funcids[0xE6] = "FUNC_ID_IO_PORT"
funcids[0xEE] = "FUNC_ID_SERIAL_API_POWER_MANAGEMENT"
funcids[0xEF] = "FUNC_ID_SERIAL_API_READY"
funcids[0xF0] = "FUNC_ID_PROPRIETARY_0"
funcids[0xF1] = "FUNC_ID_PROPRIETARY_1"
funcids[0xF2] = "FUNC_ID_PROPRIETARY_2"
funcids[0xF3] = "FUNC_ID_PROPRIETARY_3"
funcids[0xF4] = "FUNC_ID_PROPRIETARY_4"
funcids[0xF5] = "FUNC_ID_PROPRIETARY_5"
funcids[0xF6] = "FUNC_ID_PROPRIETARY_6"
funcids[0xF7] = "FUNC_ID_PROPRIETARY_7"
funcids[0xF8] = "FUNC_ID_PROPRIETARY_8"
funcids[0xF9] = "FUNC_ID_PROPRIETARY_9"
funcids[0xFA] = "FUNC_ID_PROPRIETARY_A"
funcids[0xFB] = "FUNC_ID_PROPRIETARY_B"
funcids[0xFC] = "FUNC_ID_PROPRIETARY_C"
funcids[0xFD] = "FUNC_ID_PROPRIETARY_D"
funcids[0xFE] = "FUNC_ID_PROPRIETARY_E"
funcids[0xFF] = "FUNC_ID_UNKNOWN"

def frame_tostring(frame):
    if(len(frame) < 2):
      return "EEE"
    try:
      req = int(frame[0],16)
      fid =  int(frame[1],16)
    except ValueError:
      return ""
    
    if(req==0x00):
        s="\033[34;1m REQ "
    else:
        s="\033[33;1m RES "
    
    try:
        return s + funcids[fid]+"("+hex(fid)+") "+ str(frame[2:-1]) + "\033[0m"
    except KeyError:
        return s + "Unknown(%2.2x) " % fid  + str(frame[2:-1]) + "\033[0m"
    
    None

if __name__ == '__main__':
    
    if len(sys.argv) < 2:
        print("Usage:")
        print("serial_decode_zpc.py <path-to-serial-log-file>")
        sys.exit()

    try:
        f = open(sys.argv[1], "r")
    except OSError:
        print("Could not open file:", sys.argv[1]);
        sys.exit()

    t0 = 0
    n=0
    try:
        lines = f.readlines()
    except OSError:
        print("Could not read file:", sys.argv[1]);
        sys.exit()

    for line in lines:
        t = line.split(' ')
        if "Start of new log" in line: continue

        if len(t) < 5:
          print (line)
          continue
        n=n+1
        #print t[2:-1]
       
        # ZPC style log
        try:
            hdr = "%4.4i %s %s " % (n,t[0],t[1])
        except ValueError:
            sys.exit()

        if(t[2] == "W"):
            hdr+= " TX--> "
        if(t[2] == "R"):
            hdr+= " RX<-- "

        p=3
        while(p< len(t)-1):
            try:
                cmd = int(t[p],16)
            except ValueError:
                cmd = 0xff

            if(cmd == ACK): 
                print (hdr + "ACK")
                p = p +1
            elif(cmd == CAN):
                print (hdr + "CAN")
                p = p +1
            elif(cmd == NAK):
                print (hdr + "NAK")
                p = p +1
            elif(cmd == SOF):
                try: 
                    framelen= int(t[p+1],16)
                except ValueError:
                    break;
                print (hdr + frame_tostring(t[p+2:p+2+framelen]))
                p = p + framelen + 2
            else:
                print ("BAD Token  %x" % cmd)
                p = p +1
    pass
    f.close()
