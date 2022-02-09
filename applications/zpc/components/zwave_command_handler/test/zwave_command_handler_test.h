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

#ifndef ZWAVE_COMMAND_HANDLER_TEST_H
#define ZWAVE_COMMAND_HANDLER_TEST_H

#include "zwave_controller_connection_info.h"
#include "ZW_classcmd.h"

// Declaration of our local init functions
void cc_version_init();
void cc_time_init();
void cc_zwave_plus_init();

///////////////////////////////////////////////////////////////////////////////
// Test connection_info data
///////////////////////////////////////////////////////////////////////////////
static const zwave_controller_connection_info_t connection_info_frame_non_secure
  = {.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE};

static const zwave_controller_connection_info_t connection_info_frame_security_0
  = {.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0};

static const zwave_controller_connection_info_t
  connection_info_frame_security_2_authenticated
  = {.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_AUTHENTICATED};

static const zwave_controller_connection_info_t
  connection_info_frame_security_2_access
  = {.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS};

///////////////////////////////////////////////////////////////////////////////
// Test frame payloads
///////////////////////////////////////////////////////////////////////////////
// Z-Wave Plus info Get Command.
static const uint8_t frame_data_1[]
  = {COMMAND_CLASS_ZWAVEPLUS_INFO_V2,
     ZWAVEPLUS_INFO_GET_V2,
     0x02};  //Add some more payload than the minimum

// Time Report frame
static const uint8_t frame_data_2[]
  = {COMMAND_CLASS_TIME_V2, TIME_REPORT, 0x85, 0x00, 0xFF};

// Window Covering Get Command
static const uint8_t frame_data_3[]
  = {COMMAND_CLASS_WINDOW_COVERING, WINDOW_COVERING_GET};

// User Code Set Command
static const uint8_t frame_data_4[]
  = {COMMAND_CLASS_USER_CODE,
     USER_CODE_SET,
     0x01,
     0x04,
     '1',
     '2',
     '3',
     '4'};  //Somebody tries to set a very insecure User code ! :-D

// Time Get Command.
static const uint8_t frame_data_5[] = {COMMAND_CLASS_TIME, TIME_GET};

#endif  //ZWAVE_COMMAND_HANDLER_TEST_H