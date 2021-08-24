/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef ZWAVE_RX_TEST_H
#define ZWAVE_RX_TEST_H

// Data type from zwapi (zwapi_node_info_header_t)
#include "zwapi_protocol_controller_mock.h"

// Includes from the component being tested
#include "zwave_rx.h"
#include "zwave_rx_fixt.h"

// For verifying the value of &zwave_rx_process
#include "zwave_rx_process.h"
#include "zwave_rx_internals.h"

// Test constants, they are essentially random numbers
#define ZWAVE_RX_TEST_SERIAL_PORT         "my/test/device"
#define ZWAVE_RX_TEST_MAXIMUM_POWER_DBM   10
#define ZWAVE_RX_TEST_MEASURED_0DBM_POWER 5
#define ZWAVE_RX_TEST_RF_REGION_STRING \
  "KR"  // Keep this value aligned with ZWAVE_RX_TEST_RF_REGION_INT
#define ZWAVE_RX_TEST_RF_REGION_INT       0x21
#define ZWAVE_RX_TEST_RX_STATUS           38
#define ZWAVE_RX_TEST_DESTINATION_NODE_ID 194
#define ZWAVE_RX_TEST_SOURCE_NODE_ID      51
#define ZWAVE_RX_TEST_RSSI                -102
#define ZWAVE_RX_TEST_RFD                 99

// Test frames
static const uint8_t test_expected_frame_data_1[] = {20, 34, 69, 2, 255, 86};
static const uint8_t test_expected_frame_data_2[]
  = {85, 8, 62, 57, 250, 240, 43, 1, 0};

// Test NIF 1
// This is the zwapi_node_info_header_t part of the NIF
static const zwapi_node_info_header_t expected_node_info_header_1
  = {2, 3, 0, {4, 5, 9}};
// This is the NIF without the first 2 bytes
static const uint8_t expected_node_info_array_1[] = {4, 5, 9, 8, 0x20, 0x9F};
// Converted expected_node_info_array_header_1 / expected_node_info_array_1
static const zwave_node_info_t expected_node_info_1
  = {.listening_protocol        = 2,
     .optional_protocol         = 3,
     .basic_device_class        = 4,
     .generic_device_class      = 5,
     .specific_device_class     = 9,
     .command_class_list_length = 3,
     .command_class_list        = {8, 0x20, 0x9F}};
//Converted expected_node_info_array_1, without the header first 2 bytes
static const zwave_node_info_t expected_node_info_no_header_1
  = {.listening_protocol        = 0,
     .optional_protocol         = 0,
     .basic_device_class        = 4,
     .generic_device_class      = 5,
     .specific_device_class     = 9,
     .command_class_list_length = 3,
     .command_class_list        = {8, 0x20, 0x9F}};

// Test NIF 2
// This is the NIF without the first 2 bytes
static const uint8_t expected_node_info_array_2[]
  = {0, 2, 221, 69, 111, 0x20, 0x9F, 0x98, 0x2A};
// Converted expected_node_info_array_header_2 / expected_node_info_array_2
static const zwave_node_info_t expected_node_info_2
  = {.listening_protocol        = 0,
     .optional_protocol         = 0,
     .basic_device_class        = 0,
     .generic_device_class      = 2,
     .specific_device_class     = 221,
     .command_class_list_length = 6,
     .command_class_list        = {69, 111, 0x20, 0x9F, 0x98, 0x2A}};

static const zwave_home_id_t expected_home_id_1 = 0x12345678;
static const zwave_home_id_t expected_home_id_2 = 0x58A009D2;

#endif  //ZWAVE_RX_TEST_H