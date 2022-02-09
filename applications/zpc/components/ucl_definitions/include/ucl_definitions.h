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
 * @defgroup ucl_definitions UCL definitions
 * @ingroup zpc_components
 * @brief  Type definitions related to the UCL language.
 *
 * This interface module provide all the required type definitions relating to
 * the UCL language.
 *
 * @{
 */

#ifndef UCL_DEFINITIONS_H
#define UCL_DEFINITIONS_H

#include "uic_enum.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Possible NetworkState values for a node in the State topic.
 */
typedef enum {
  /// Node ID assigned but not included yet. Only used for progress reporting
  /// during inclusion.
  ///
  NODE_STATE_TOPIC_STATE_NODEID_ASSIGNED,
  /// 	Included and ready to be operated
  ///
  NODE_STATE_TOPIC_STATE_INCLUDED,
  /// Interview of the node has failed. Node cannot be operated
  ///
  NODE_STATE_TOPIC_STATE_INTERVIEW_FAIL,
  /// Node is currently being interviewed by the ZPC
  ///
  NODE_STATE_TOPIC_INTERVIEWING,
  /// Has previously been operating, but seems to be offline right now.
  /// Ie, node is out of battery or out of reach (i.e. Failing)
  NODE_STATE_TOPIC_STATE_OFFLINE,
  /// The protocol controller servicing the node is currently offline.
  ///
  NODE_STATE_TOPIC_STATE_UNAVAILABLE,
  /// This element must always be last
  ///
  NODE_STATE_TOPIC_LAST,
} UIC_ENUM8(node_state_topic_state_t);

#ifdef __cplusplus
}
#endif

#endif  //UCL_DEFINITIONS_H
/** @} end ucl_definitions */
