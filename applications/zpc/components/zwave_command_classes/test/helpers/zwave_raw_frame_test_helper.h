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

#ifndef ZWAVE_RAW_FRAME_TEST_HELPER_H
#define ZWAVE_RAW_FRAME_TEST_HELPER_H

#include "sl_status.h"
#include "zwave_controller_connection_info.h"
#include "zwave_rx.h"
#include "attribute_store_mock.h"

typedef sl_status_t (*handler)(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length);

void execute_frame_expect_result(handler command_handler,
                                 const uint8_t *command_frame,
                                 const uint16_t command_frame_length,
                                 const sl_status_t expected_status);

void execute_frame_expect_frame(handler command_handler,
                                const uint8_t *command_frame,
                                const uint16_t command_frame_length,
                                const uint8_t *expected_frame,
                                const uint16_t expected_frame_length,
                                const sl_status_t expected_status);

void assert_dissallow_multicast(handler command_handler, uint8_t cc_cmd[2]);

/**
 * @brief expect an (unsolicited) report frame sent using
 * zwave_command_class_send_report() and verify that the payload
 * matches expectation
 */
void expect_report_frame(const uint8_t *expected_frame,
                         const uint16_t expected_frame_length);

void is_zwave_command_class_filtered_for_root_device_verification(
  zwave_command_class_t command_class, attribute_store_node_t updated_node);
#endif
