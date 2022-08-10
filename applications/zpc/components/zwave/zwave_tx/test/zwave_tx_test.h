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

#ifndef ZWAVE_TX_TEST_H
#define ZWAVE_TX_TEST_H

#include "zwave_controller_connection_info.h"
#include "zwave_tx_groups_mock.h"

// Component to test
#include "zwave_tx_process.h"
#include "zwave_tx.h"

#define DEFAULT_CONTIKI_CLOCK_JUMP    123
#define TX_BACKOFF_CONTIKI_CLOCK_JUMP 10000
#define EMERGENCY_TIMEOUT             70 * 1000  // SEND_DATA_EMERGENCY_TIMEOUT 65

// Test frames payload
static const uint8_t test_expected_frame_data_1[] = {20, 34, 69, 2, 255, 86};
static const uint8_t test_expected_frame_data_2[]
  = {85, 8, 62, 57, 250, 240, 43, 1, 0};
static const uint8_t test_expected_frame_data_3[]
  = {5, 0x9F, 2, 255, 250, 42, 43, 1, 4, 5, 48, 52, 85};

// Do not reuse this in any of the test_connections (.remote.node_id)
static const zwave_node_id_t our_node_id = 42;

// test connection_info structures
static const zwave_controller_connection_info_t test_connection_1 = {
  .local         = {.node_id = 1, .endpoint_id = 0, .is_multicast = false},
  .remote        = {.node_id = 3, .endpoint_id = 2, .is_multicast = false},
  .encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
};

static const zwave_controller_connection_info_t test_connection_2 = {
  .local         = {.node_id = 0, .endpoint_id = 0, .is_multicast = false},
  .remote        = {.node_id = 4, .endpoint_id = 2, .is_multicast = false},
  .encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
};

static const zwave_controller_connection_info_t test_connection_3 = {
  .local         = {.node_id = 10, .endpoint_id = 10, .is_multicast = true},
  .remote        = {.node_id = 5, .endpoint_id = 0, .is_multicast = false},
  .encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
};

static const zwave_tx_options_t test_tx_options_1
  = {.number_of_responses = 0,
     .discard_timeout_ms  = 0,
     .qos_priority        = 0xFFFF,
     .fasttrack           = true,
     .send_follow_ups     = false,
     .transport           = {
                 .group_id           = ZWAVE_TX_INVALID_GROUP,
                 .rf_power           = NORMAL_POWER,
                 .is_first_follow_up = false,
                 .is_test_frame      = false,
     }};

static const zwave_tx_options_t test_tx_options_2
  = {.number_of_responses = 1,
     .discard_timeout_ms  = 400,
     .qos_priority        = 20,
     .fasttrack           = false,
     .send_follow_ups     = false,
     .transport           = {
                 .is_first_follow_up = false,
                 .is_test_frame      = false,
                 .rf_power           = NORMAL_POWER,
                 .group_id           = ZWAVE_TX_INVALID_GROUP,
     }};

static const zwave_tx_options_t test_tx_options_2_multi_responses
  = {.number_of_responses = 3,
     .discard_timeout_ms  = 400,
     .qos_priority        = 20,
     .fasttrack           = false,
     .send_follow_ups     = false,
     .transport           = {
                 .is_first_follow_up = false,
                 .is_test_frame      = false,
                 .rf_power           = NORMAL_POWER,
                 .group_id           = ZWAVE_TX_INVALID_GROUP,
     }};

static const zwave_tx_options_t test_tx_options_3
  = {.number_of_responses = 0,
     .discard_timeout_ms  = 0,
     .qos_priority        = 10,
     .fasttrack           = false,
     .send_follow_ups     = false,
     .transport           = {
                 .is_test_frame      = false,
                 .rf_power           = NORMAL_POWER,
                 .group_id           = ZWAVE_TX_INVALID_GROUP,
                 .is_first_follow_up = false,
     }};

static const zwave_tx_options_t test_tx_options_test_frame
  = {.number_of_responses = 1,
     .discard_timeout_ms  = 0,
     .qos_priority        = 4356,
     .fasttrack           = false,
     .send_follow_ups     = false,
     .transport           = {
                 .is_first_follow_up = false,
                 .is_test_frame      = true,
                 .rf_power           = MINUS_3_DBM,
                 .group_id           = ZWAVE_TX_INVALID_GROUP,
     }};

static const zwave_tx_options_t test_tx_options_group_id
  = {.number_of_responses = 1,
     .discard_timeout_ms  = 0,
     .qos_priority        = 0xFFFF,
     .fasttrack           = false,
     .send_follow_ups     = false,
     .transport           = {
                 .is_first_follow_up = false,
                 .is_test_frame      = false,
                 .rf_power           = NORMAL_POWER,
                 .group_id           = 34,
     }};

static const uint8_t expected_tx_options_singlecast
  = (TRANSMIT_OPTION_ACK | TRANSMIT_OPTION_AUTO_ROUTE
     | TRANSMIT_OPTION_EXPLORE);
static const uint8_t expected_tx_options_multicast = 0;
static const uint8_t expected_tx_options_multicast_as_broadcast
  = TRANSMIT_OPTION_MULTICAST_AS_BROADCAST;
static const uint8_t expected_tx_options_singlecast_fasttrack
  = TRANSMIT_OPTION_ACK;

#endif  //ZWAVE_TX_TEST_H