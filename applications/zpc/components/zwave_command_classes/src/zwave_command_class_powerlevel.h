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

#ifndef ZWAVE_COMMAND_CLASS_POWERLEVEL_H
#define ZWAVE_COMMAND_CLASS_POWERLEVEL_H

#include "sl_status.h"
#include "zwave_rx.h"
#include "zwapi_protocol_basis.h"

#include "ctimer.h"
#include "ZW_classcmd.h"

/**
 * Possible values for Powerlevel test status
 */
typedef enum {
  ///< None of the frames were acknowdledged by the destination
  TEST_FAILED = POWERLEVEL_TEST_NODE_REPORT_ZW_TEST_FAILED,
  ///< Test completed and at least one frame was acknowledged by the destination
  TEST_SUCCESSFUL = POWERLEVEL_TEST_NODE_REPORT_ZW_TEST_SUCCES,
  ///< Test is ongoing
  TEST_IN_PROGRESS = POWERLEVEL_TEST_NODE_REPORT_ZW_TEST_INPROGRESS,
} power_level_test_status_t;

/**
 * Settings and state of powerlevel test.
 */
typedef struct power_level_test {
  ///< NodeID that asked us to perform a powerlevel test
  zwave_controller_connection_info_t connection;
  ///< Destination NodeID for the power level test
  zwave_node_id_t destination_node;
  ///< Powerlevel to use for the test
  rf_power_level_t power_level;
  ///< Number of frames to left transmit for the test
  uint16_t frame_count;
  ///< Number of frames that have been acknowledged
  uint16_t acknowledged_frames_count;
  ///< Status of the power level test
  power_level_test_status_t status;
} power_level_test_t;

/**
 * Powerlevel configuration
 */
typedef struct power_level_setting {
  /// Current power level setting.
  rf_power_level_t power_level;
  /// ctimer used to rollback the power to normal.
  struct ctimer timer;
  /// Timestamp of when the powerlevel will is set back to normal.
  /// 0 if we are already at the normal level.
  clock_time_t expiration_time;
} powerlevel_setting_t;

/**
 * @brief setup for powerelvel command class
 *
 * This setup will register the time command handler
 * to the Z-Wave CC framework,
 *
 * @returns SL_STATUS_OK if successful
 * @returns SL_STATUS_FAIL if an error occurred
 */
sl_status_t zwave_command_class_powerlevel_init(void);

/**
 * @brief Handles incoming powerlevel commands
 *
 * @param connection_info Info about the connection properties of this frame.
 * @param frame_data The data payload of this frame.
 * @param frame_length The length of this frame.
 * @returns sl_status_t representing the outcome of receiving the frame
 *          In case where the command is controlled, it should be set to
 *          SL_STATUS_OK, even when encountering some parsing errors
 *          In case where the command is supported, it should be set to the
 *          @ref sl_status_t corresponding to the correct time Status code.
 *          Refer to @ref zwave_command_handler_t
 */
sl_status_t zwave_command_class_powerlevel_support_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length);

#endif  //ZWAVE_COMMAND_CLASS_POWERLEVEL_H
