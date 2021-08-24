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

#ifndef ZCL_PROFILES_COMMAND_FIELD_EXTRACT_H
#define ZCL_PROFILES_COMMAND_FIELD_EXTRACT_H

#include "zigpc_common_zigbee.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Parse and build the ZCL frame using the command fields passed in.
 * This function will parse the informing MQTT message for ZCL commands and
 * extract any known command fields that is associated with the command (ex.
 * the fields: Level, TransitionTime, OptionsMask, OptionsOverride associated
 * with the Level cluster command 'MoveToLevel').
 *
 * @param message             Unify command JSON payload buffer.
 * @param message_length      Unify command JSON payload size.
 * @param command_field_count Number of fields to parse.
 * @param command_fields      ZCL command field information as a list.
 * @param zcl_frame           ZCL frame to populate based on information received
 * @return sl_status_t
 */
sl_status_t zcl_profiles_parse_build_command_fields(
  const char *message,
  size_t message_length,
  size_t command_field_count,
  const zigpc_zcl_profiles_command_field_t *command_fields,
  zcl_frame_t *const zcl_frame);

#ifdef __cplusplus
}
#endif

#endif /* ZCL_PROFILES_COMMAND_FIELD_EXTRACT_H */
