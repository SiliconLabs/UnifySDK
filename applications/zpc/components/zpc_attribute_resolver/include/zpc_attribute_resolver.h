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
 * @defgroup zpc_attribute_resolver ZPC Attribute Resolver
 * @ingroup zpc_components
 * @brief ZPC part of the attribute resolver
 *
 * The ZPC Attribute resolver is in charge of sending frames and listening for Send
 * data callbacks in the context of resolving attributes. It then relays
 * resolution information to the @ref attribute_resolver
 *
 * @{
 */

#ifndef ZPC_ATTRIBUTE_RESOLVER_H
#define ZPC_ATTRIBUTE_RESOLVER_H

#include "attribute_store.h"
#include "attribute_resolver_rule.h"

typedef enum {
  /// Frame was delivered to the node without supervision.
  FRAME_SENT_EVENT_OK_NO_SUPERVISION,
  /// Frame was delivered to the node with Supervision,
  /// and it responded with SUPERVISION_REPORT_SUCCESS.
  FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS,
  /// Frame was delivered to the node with Supervision,
  /// and it responded with SUPERVISION_REPORT_NO_SUPPORT.
  FRAME_SENT_EVENT_OK_SUPERVISION_NO_SUPPORT,
  /// Frame was delivered to the node with Supervision,
  /// and it responded with SUPERVISION_REPORT_WORKING.
  FRAME_SENT_EVENT_OK_SUPERVISION_WORKING,
  /// Frame was delivered to the node with Supervision,
  /// and it responded with SUPERVISION_REPORT_FAIL.
  FRAME_SENT_EVENT_OK_SUPERVISION_FAIL,
  /// Frame was not delivered to the node.
  FRAME_SENT_EVENT_FAIL,
} zpc_resolver_event_t;

typedef void (*zpc_resolver_event_notification_function_t)(
  attribute_store_node_t node,
  resolver_rule_type_t rule_type,
  zpc_resolver_event_t event);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Registers a custom function for sent events. The ZPC Resolver will
 * call this function and will not execute the default sent data complete
 * handling for a given type when a handler is registered.
 *
 * @param type      The attribute store type for which the custom handler will take care
 * @param function  The function to invoke when a send data complete event occurred.
 *
 * @returns sl_status_t indicating the outcome of the registration.
 *          Only one handler can be registered at a time.
 */
sl_status_t register_send_event_handler(
  attribute_store_type_t type,
  zpc_resolver_event_notification_function_t function);

/**
 * @brief unregisters a custom function for sent events.
 *
 * @param type      The attribute store type for the custom handler
 * @param function  The function that was registered as a handler.
 *
 * @returns SL_STATUS_OK
 */
sl_status_t unregister_send_event_handler(
  attribute_store_type_t type,
  zpc_resolver_event_notification_function_t function);

#ifdef __cplusplus
}
#endif

#endif  //ZPC_ATTRIBUTE_RESOLVER_H
/** @} end zpc_attribute_resolver */
