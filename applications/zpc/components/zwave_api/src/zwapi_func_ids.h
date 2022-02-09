/******************************************************************************
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
 * @file zwapi_func_ids.h
 * @brief List of function IDs used by the Z-Wave module. It contains the
 * FUNC_ID definitions provided by the Z-Wave protocol SDK
 *
 */

#ifndef ZWAPI_FUNC_IDS_H
#define ZWAPI_FUNC_IDS_H

/* Function IDs */
#define FUNC_ID_SERIAL_API_GET_INIT_DATA         0x02
#define FUNC_ID_SERIAL_API_APPL_NODE_INFORMATION 0x03
#define FUNC_ID_APPLICATION_COMMAND_HANDLER      0x04
#define FUNC_ID_ZW_GET_CONTROLLER_CAPABILITIES   0x05

/* SERIAL API ver 4 added - START */
#define FUNC_ID_SERIAL_API_SET_TIMEOUTS     0x06
#define FUNC_ID_SERIAL_API_GET_CAPABILITIES 0x07
#define FUNC_ID_SERIAL_API_SOFT_RESET       0x08
/* SERIAL API ver 4 added - END */

#define FUNC_ID_ZW_GET_PROTOCOL_VERSION 0x09

/* Function ID for startup message */
#define FUNC_ID_SERIAL_API_STARTED 0x0A
#define FUNC_ID_SERIAL_API_SETUP   0x0B

#define FUNC_ID_ZW_SET_RF_RECEIVE_MODE   0x10
#define FUNC_ID_ZW_SET_SLEEP_MODE        0x11
#define FUNC_ID_ZW_SEND_NODE_INFORMATION 0x12
#define FUNC_ID_ZW_SEND_DATA             0x13
#define FUNC_ID_ZW_SEND_DATA_MULTI       0x14
#define FUNC_ID_ZW_GET_VERSION           0x15

/* SERIAL API ver 4 added - START */
#define FUNC_ID_ZW_SEND_DATA_ABORT    0x16
#define FUNC_ID_ZW_RF_POWER_LEVEL_SET 0x17
#define FUNC_ID_ZW_SEND_DATA_META     0x18
/* SERIAL API ver 4 added - END */

#define FUNC_ID_ZW_RESERVED_SD  0x19
#define FUNC_ID_ZW_RESERVED_SDM 0x1A
#define FUNC_ID_ZW_RESERVED_SRI 0x1B

#define FUNC_ID_ZW_SET_ROUTING_INFO 0x1B

#define FUNC_ID_ZW_GET_RANDOM                     0x1C
#define FUNC_ID_ZW_RANDOM                         0x1D
#define FUNC_ID_ZW_RF_POWER_LEVEL_REDISCOVERY_SET 0x1E

#define FUNC_ID_MEMORY_GET_ID     0x20
#define FUNC_ID_MEMORY_GET_BYTE   0x21
#define FUNC_ID_MEMORY_PUT_BYTE   0x22
#define FUNC_ID_MEMORY_GET_BUFFER 0x23
#define FUNC_ID_MEMORY_PUT_BUFFER 0x24
/* Unimplemented - START */
#define FUNC_ID_SERIAL_API_GET_APPL_HOST_MEMORY_OFFSET 0x25
#define FUNC_ID_DEBUG_OUTPUT                           0x26
/* Unimplemented - END */

#define FUNC_ID_AUTO_PROGRAMMING 0x27

#define FUNC_ID_NVR_GET_VALUE 0x28

#define FUNC_ID_NVM_GET_ID                0x29
#define FUNC_ID_NVM_EXT_READ_LONG_BUFFER  0x2A
#define FUNC_ID_NVM_EXT_WRITE_LONG_BUFFER 0x2B
#define FUNC_ID_NVM_EXT_READ_LONG_BYTE    0x2C
#define FUNC_ID_NVM_EXT_WRITE_LONG_BYTE   0x2D
#define FUNC_ID_NVM_BACKUP_RESTORE        0x2E

#define FUNC_ID_CLOCK_SET        0x30
#define FUNC_ID_CLOCK_GET        0x31
#define FUNC_ID_CLOCK_CMP        0x32
#define FUNC_ID_RTC_TIMER_CREATE 0x33
#define FUNC_ID_RTC_TIMER_READ   0x34
#define FUNC_ID_RTC_TIMER_DELETE 0x35
#define FUNC_ID_RTC_TIMER_CALL   0x36

#define FUNC_ID_CLEAR_TX_TIMERS 0x37
#define FUNC_ID_GET_TX_TIMERS   0x38

#define FUNC_ID_ZW_GET_BACKGROUND_RSSI         0x3B
#define FUNC_ID_ZW_REMOVE_NODE_ID_FROM_NETWORK 0x3F

#define FUNC_ID_ZW_SET_LEARN_NODE_STATE         0x40
#define FUNC_ID_ZW_GET_NODE_PROTOCOL_INFO       0x41
#define FUNC_ID_ZW_SET_DEFAULT                  0x42
#define FUNC_ID_ZW_NEW_CONTROLLER               0x43
#define FUNC_ID_ZW_REPLICATION_COMMAND_COMPLETE 0x44
#define FUNC_ID_ZW_REPLICATION_SEND_DATA        0x45
#define FUNC_ID_ZW_ASSIGN_RETURN_ROUTE          0x46
#define FUNC_ID_ZW_DELETE_RETURN_ROUTE          0x47
#define FUNC_ID_ZW_REQUEST_NODE_NEIGHBOR_UPDATE 0x48
#define FUNC_ID_ZW_APPLICATION_UPDATE           0x49

/*Obsolete use ZW_APPLICATION_UPDATE */
#define FUNC_ID_ZW_APPLICATION_CONTROLLER_UPDATE 0x49

#define FUNC_ID_ZW_ADD_NODE_TO_NETWORK      0x4A
#define FUNC_ID_ZW_REMOVE_NODE_FROM_NETWORK 0x4B
#define FUNC_ID_ZW_CREATE_NEW_PRIMARY       0x4C
#define FUNC_ID_ZW_CONTROLLER_CHANGE        0x4D

#define FUNC_ID_ZW_RESERVED_FN 0x4E
#define FUNC_ID_ZW_RESERVED_AR 0x4F

/* End Nodes only */
#define FUNC_ID_ZW_SET_LEARN_MODE 0x50
/* End Nodes only end */

#define FUNC_ID_ZW_ASSIGN_SUC_RETURN_ROUTE 0x51
#define FUNC_ID_ZW_ENABLE_SUC              0x52
#define FUNC_ID_ZW_REQUEST_NETWORK_UPDATE  0x53
#define FUNC_ID_ZW_SET_SUC_NODE_ID         0x54
#define FUNC_ID_ZW_DELETE_SUC_RETURN_ROUTE 0x55
#define FUNC_ID_ZW_GET_SUC_NODE_ID         0x56
#define FUNC_ID_ZW_SEND_SUC_ID             0x57

#define FUNC_ID_ZW_RESERVED_ASR 0x58
#define FUNC_ID_ZW_RESERVED_RN  0x59

#define FUNC_ID_ZW_REQUEST_NODE_NEIGHBOR_UPDATE_OPTION 0x5A

/* End Nodes only */
#define FUNC_ID_ZW_SUPPORT9600_ONLY 0x5B
/* End Nodes only end */

/* Enhanced/Routing End Nodes only */
#define FUNC_ID_ZW_REQUEST_NEW_ROUTE_DESTINATIONS 0x5C
#define FUNC_ID_ZW_IS_NODE_WITHIN_DIRECT_RANGE    0x5D
/* Enhanced/Routing End Nodes only end */

#define FUNC_ID_ZW_EXPLORE_REQUEST_INCLUSION 0x5E
#define FUNC_ID_ZW_EXPLORE_REQUEST_EXCLUSION 0x5F

#define FUNC_ID_ZW_REQUEST_NODE_INFO     0x60
#define FUNC_ID_ZW_REMOVE_FAILED_NODE_ID 0x61
#define FUNC_ID_ZW_IS_FAILED_NODE_ID     0x62
#define FUNC_ID_ZW_REPLACE_FAILED_NODE   0x63

/* In 6.0x the function id was wrong so we need to support this wrong function id as well in the future */
#define FUNC_ID_ZW_SET_ROUTING_MAX_6_00 0x65

#define FUNC_ID_ZW_IS_PRIMARY_CTRL 0x66

#define FUNC_ID_ZW_AES_ECB 0x67

#define FUNC_ID_TIMER_START   0x70
#define FUNC_ID_TIMER_RESTART 0x71
#define FUNC_ID_TIMER_CANCEL  0x72
#define FUNC_ID_TIMER_CALL    0x73

/* Firmware Update API */
#define FUNC_ID_ZW_FIRMWARE_UPDATE_NVM 0x78

/* Installer API */
#define FUNC_ID_GET_ROUTING_TABLE_LINE 0x80
#define FUNC_ID_GET_TX_COUNTER         0x81
#define FUNC_ID_RESET_TX_COUNTER       0x82
#define FUNC_ID_STORE_NODEINFO         0x83
#define FUNC_ID_STORE_HOMEID           0x84
/* Installer API only end */

#define FUNC_ID_LOCK_ROUTE_RESPONSE 0x90
#ifdef ZW_ROUTING_DEMO
#define FUNC_ID_ZW_SEND_DATA_ROUTE_DEMO 0x91
#endif

#define FUNC_ID_ZW_GET_LAST_WORKING_ROUTE 0x92
#define FUNC_ID_ZW_SET_LAST_WORKING_ROUTE 0x93

#define FUNC_ID_SERIAL_API_TEST 0x95

#define FUNC_ID_SERIAL_API_EXT 0x98

/* ZW_CONTROLLER_BRIDGE only START */
#define FUNC_ID_SERIAL_API_APPL_VIRTUAL_NODE_INFORMATION 0xA0
/* OBSOLETE: In DevKit 4.5x/6.0x Controller Bridge applications, this is obsoleted */
/* by the FUNC_ID_APPLICATION_COMMAND_HANDLER_BRIDGE */
#define FUNC_ID_APPLICATION_VIRTUAL_NODE_COMMAND_HANDLER 0xA1
#define FUNC_ID_ZW_SEND_VIRTUAL_NODE_INFORMATION         0xA2
#define FUNC_ID_ZW_SEND_VIRTUAL_NODE_DATA                0xA3
#define FUNC_ID_ZW_SET_VIRTUAL_NODE_TO_LEARN_MODE        0xA4
#define FUNC_ID_ZW_GET_VIRTUAL_NODES                     0xA5
#define FUNC_ID_ZW_IS_VIRTUAL_NODE                       0xA6
#define FUNC_ID_ZW_RESERVED_SSD                          0xA7
/* DevKit 4.5x/6.0x added - obsoletes FUNC_ID_APPLICATION_VIRTUAL_NODE_COMMAND_HANDLER and */
/* FUNC_ID_APPLICATION_COMMAND_HANDLER for the Controller Bridge applications as */
/* this handles both cases - only for 4.5x/6.0x based Controller Bridge applications */
#define FUNC_ID_APPLICATION_COMMAND_HANDLER_BRIDGE 0xA8
/* DevKit 4.5x/6.0x added - Adds sourceNodeID to the parameter list */
#define FUNC_ID_ZW_SEND_DATA_BRIDGE       0xA9
#define FUNC_ID_ZW_SEND_DATA_META_BRIDGE  0xAA
#define FUNC_ID_ZW_SEND_DATA_MULTI_BRIDGE 0xAB
/* ZW_CONTROLLER_BRIDGE only END */

#define FUNC_ID_PWR_SETSTOPMODE    0xB0  // ZW102 only
#define FUNC_ID_PWR_CLK_PD         0xB1  // ZW102 only
#define FUNC_ID_PWR_CLK_PUP        0xB2  // ZW102 only
#define FUNC_ID_PWR_SELECT_CLK     0xB3  // ZW102 only
#define FUNC_ID_ZW_SET_WUT_TIMEOUT 0xB4  // ZW201 only
#define FUNC_ID_ZW_IS_WUT_KICKED   0xB5  // ZW201 only

#define FUNC_ID_ZW_WATCHDOG_ENABLE  0xB6
#define FUNC_ID_ZW_WATCHDOG_DISABLE 0xB7
#define FUNC_ID_ZW_WATCHDOG_KICK    0xB8
/* Obsolete use FUNC_ID_ZW_INT_EXT_LEVEL_SET */
#define FUNC_ID_ZW_SET_EXT_INT_LEVEL 0xB9  // ZW201 only
#define FUNC_ID_ZW_INT_EXT_LEVEL_SET 0xB9

#define FUNC_ID_ZW_RF_POWER_LEVEL_GET   0xBA
#define FUNC_ID_ZW_GET_NEIGHBOR_COUNT   0xBB
#define FUNC_ID_ZW_ARE_NODES_NEIGHBORS  0xBC

#define FUNC_ID_ZW_TYPE_LIBRARY        0xBD
#define FUNC_ID_ZW_SEND_TEST_FRAME     0xBE
#define FUNC_ID_ZW_GET_PROTOCOL_STATUS 0xBF

#define FUNC_ID_ZW_SET_PROMISCUOUS_MODE 0xD0
/* SERIAL API ver 5 added - START */
#define FUNC_ID_PROMISCUOUS_APPLICATION_COMMAND_HANDLER 0xD1
/* SERIAL API ver 5 added - END */

#define FUNC_ID_ZW_WATCHDOG_START 0xD2
#define FUNC_ID_ZW_WATCHDOG_STOP  0xD3

#define FUNC_ID_ZW_SET_ROUTING_MAX 0xD4
/* Unimplemented - START */
/* Obsoleted */
#define FUNC_ID_ZW_GET_ROUTING_MAX 0xD5
/* Unimplemented - END */

#define FUNC_ID_SERIAL_API_GET_LR_NODES 0xDA
#define FUNC_ID_GET_LR_CHANNEL          0xDB
#define FUNC_ID_SET_LR_CHANNEL          0xDC
#define FUNC_ID_ZW_SET_LR_VIRTUAL_IDS   0xDD

/* Allocated for NUNIT test */
#define FUNC_ID_ZW_NUNIT_CMD  0xE0
#define FUNC_ID_ZW_NUNIT_INIT 0xE1
#define FUNC_ID_ZW_NUNIT_LIST 0xE2
#define FUNC_ID_ZW_NUNIT_RUN  0xE3
#define FUNC_ID_ZW_NUNIT_END  0xE4

#define FUNC_ID_IO_PORT_STATUS 0xE5
#define FUNC_ID_IO_PORT        0xE6

#define FUNC_ID_ENABLE_RADIO_PTI 0xE7
#define FUNC_ID_GET_RADIO_PTI    0xE8

#define FUNC_ID_SEND_NOP 0xE9

/* Allocated for Power Management */
#define FUNC_ID_SERIAL_API_POWER_MANAGEMENT 0xEE
#define FUNC_ID_SERIAL_API_READY            0xEF

/* Allocated for proprietary serial API commands */
#define FUNC_ID_PROPRIETARY_0 0xF0
#define FUNC_ID_PROPRIETARY_1 0xF1
#define FUNC_ID_PROPRIETARY_2 0xF2
#define FUNC_ID_PROPRIETARY_3 0xF3
#define FUNC_ID_PROPRIETARY_4 0xF4
#define FUNC_ID_PROPRIETARY_5 0xF5
#define FUNC_ID_PROPRIETARY_6 0xF6
#define FUNC_ID_PROPRIETARY_7 0xF7
#define FUNC_ID_PROPRIETARY_8 0xF8
#define FUNC_ID_PROPRIETARY_9 0xF9
#define FUNC_ID_PROPRIETARY_A 0xFA
#define FUNC_ID_PROPRIETARY_B 0xFB
#define FUNC_ID_PROPRIETARY_C 0xFC
#define FUNC_ID_PROPRIETARY_D 0xFD
#define FUNC_ID_PROPRIETARY_E 0xFE

/* Illegal function ID */
#define FUNC_ID_UNKNOWN 0xFF

#endif  // ZWAPI_FUNC_IDS_H
