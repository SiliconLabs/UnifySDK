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
 * @defgroup zwave_command_class_supervision_internals Supervision internals
 * @ingroup zwave_command_class_supervision
 * @brief Private types and definitions for the Supervision Command Class
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_SUPERVISION_INTERNALS_H
#define ZWAVE_COMMAND_CLASS_SUPERVISION_INTERNALS_H

#include "zwave_utils.h"  // for ZWAVE_MAX_FRAME_SIZE

// Helpers to parse incoming frames
#define SUPERVISION_GET_SESSION_ID_INDEX                   2
#define SUPERVISION_GET_ENCAPSULATED_COMMANDS_LENGTH_INDEX 3

#define SUPERVISION_REPORT_MORE_STATUS_INDEX 2
#define SUPERVISION_REPORT_SESSION_ID_INDEX  2
#define SUPERVISION_REPORT_STATUS_INDEX      3
#define SUPERVISION_REPORT_DURATION_INDEX    4

// Missing defines from ZW_Classcmd.h
#define SUPERVISION_VERSION_V2                          0x02
#define SUPERVISION_REPORT_PROPERTIES1_WAKE_UP_BIT_MASK 0x40

// We allow to encapsulate frame as big as "ZWAVE_MAX_FRAME_SIZE"
#define SUPERVISION_ENCAPSULATED_COMMAND_MAXIMUM_SIZE ZWAVE_MAX_FRAME_SIZE

// Default session duration when nodes report Unknown duration
#define MINUTE_IN_MS                         60 * 100
#define SUPERVISION_DEFAULT_SESSION_DURATION 1 * MINUTE_IN_MS

// Unique ID/handle associated to each supervision session.
typedef uint32_t supervision_id_t;
#define INVALID_SUPERVISION_ID ((supervision_id_t)0)

/**
 * Object containing tracking information for a Supervision Session
 */
typedef struct zwave_supervision_get_frame {
  ///> Command Class identifier
  uint8_t command_class;
  ///> Command identifier
  uint8_t command;
  ///> Status Updates / Wake up Request / Session ID
  uint8_t status_wake_session_field;
  ///> length of the encapsulated command
  uint8_t encapsulated_command_length;
  ///> Encapsulated command
  uint8_t encapsulated_command[SUPERVISION_ENCAPSULATED_COMMAND_MAXIMUM_SIZE];
} zwave_supervision_get_frame_t;

/**
 * Object containing tracking information for a Supervision Session
 * that the ZPC carries out
 */
typedef struct supervision_session {
  ///< Session ID passed as part of the Supervision Command
  uint8_t session_id;
  ///< NodeID associated with the Session ID
  zwave_node_id_t node_id;
  ///< Endpoint associated with the Session ID
  zwave_endpoint_id_t endpoint_id;
  ///< Multicast Group ID used in Multicast and Singlecast follow-up frames
  zwave_multicast_group_id_t group_id;
} supervision_session_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_SUPERVISION_INTERNALS_H
/** @} end zwave_command_class_supervision_internals */
