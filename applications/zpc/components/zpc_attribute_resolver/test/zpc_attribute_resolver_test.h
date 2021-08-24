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
 * @file zpc_attribute_resolver_test.h
 * @brief Constant / defines for the ZPC attribute resolver test.
 *
 * @{
 */

#ifndef ZPC_ATTRIBUTE_RESOLVER_TEST_H
#define ZPC_ATTRIBUTE_RESOLVER_TEST_H

#include "zwave_utils.h"
#include "zwave_controller_connection_info.h"
#include "zwave_tx.h"
#include "zwave_utils.h"

zwave_node_id_t zwave_node_id_1 = 1;
zwave_node_id_t zwave_node_id_2 = 2;
zwave_node_id_t zwave_node_id_3 = 3;
zwave_node_id_t zwave_node_id_4 = 4;
zwave_node_id_t zwave_node_id_5 = 5;

zwave_endpoint_id_t zwave_endpoint_id_1 = 1;
zwave_endpoint_id_t zwave_endpoint_id_2 = 2;
zwave_endpoint_id_t zwave_endpoint_id_3 = 3;
zwave_endpoint_id_t zwave_endpoint_id_4 = 4;
zwave_endpoint_id_t zwave_endpoint_id_5 = 5;

zwave_controller_encapsulation_scheme_t encapsulation_1
  = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0;
zwave_controller_encapsulation_scheme_t encapsulation_2
  = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;

zwave_protocol_t protocol_1       = PROTOCOL_ZWAVE;
zwave_protocol_t protocol_2       = PROTOCOL_ZWAVE_LONG_RANGE;
zwave_protocol_t protocol_unknown = PROTOCOL_UNKNOWN;

zwave_controller_connection_info_t connection_info_1
  = {.local         = {.node_id = 2, .endpoint_id = 5, .is_multicast = false},
     .remote        = {.node_id = 7, .endpoint_id = 3, .is_multicast = false},
     .encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE};

zwave_controller_connection_info_t connection_info_2
  = {.local         = {.node_id = 4, .endpoint_id = 27, .is_multicast = false},
     .remote        = {.node_id = 3, .endpoint_id = 0, .is_multicast = true},
     .encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS};

zwave_tx_options_t tx_options_1 = {.number_of_responses = 0,
                                   .discard_timeout_ms  = 0,
                                   .qos_priority        = 0xFFFF,
                                   .fasttrack           = true,
                                   .is_test_frame       = false,
                                   .rf_power            = NORMAL_POWER,
                                   .group_id            = 25,
                                   .is_first_follow_up  = false,
                                   .send_follow_ups     = false};

zwave_tx_options_t tx_options_2 = {.number_of_responses = 1,
                                   .discard_timeout_ms  = 400,
                                   .qos_priority        = 20,
                                   .fasttrack           = false,
                                   .is_test_frame       = false,
                                   .rf_power            = NORMAL_POWER,
                                   .group_id            = 36,
                                   .is_first_follow_up  = false,
                                   .send_follow_ups     = false};

zwave_multicast_group_id_t group_id_1 = 39;

const uint8_t test_frame_data[] = {0x01, 0x01, 0xFF, 0x00, 0x0A, 0x04};

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif  //ZPC_ATTRIBUTE_RESOLVER_TEST_H
/** @} end zpc_attribute_resolver_test */
