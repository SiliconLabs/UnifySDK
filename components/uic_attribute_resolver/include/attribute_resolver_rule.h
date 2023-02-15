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
 * @file attribute_resolver_rule.h
 * @brief This module defines necessary API for using resolver
 * @ingroup attribute_resolver
 * @{
 */

#ifndef ATTRIBUTE_RESOLVER_RULE_H
#define ATTRIBUTE_RESOLVER_RULE_H

#include "attribute_store.h"
#include "attribute_resolver.h"
#include "clock.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Status code for the frame transmission
 *
 */
typedef enum {
  /// Frame was delivered to the target, but we don't know if the command was
  /// executed on application level
  RESOLVER_SEND_STATUS_OK,
  /// Frame delivery to target succeeded but the target needs time to
  /// Reach the Desired value.
  ///
  RESOLVER_SEND_STATUS_OK_EXECUTION_PENDING,
  /// Frame delivery to target failed
  ///
  RESOLVER_SEND_STATUS_FAIL,
  /// Frame was delivered to target and we know for sure that the operation has
  /// been accepted, ie. the light is actually on now
  RESOLVER_SEND_STATUS_OK_EXECUTION_VERIFIED,
  /// Frame was delivered to target and it either does not support
  /// the operation or it failed trying
  RESOLVER_SEND_STATUS_OK_EXECUTION_FAILED,
  /// A custom handler took care of parsing the status and adjusting the
  /// attribute store values.
  RESOLVER_SEND_STATUS_ALREADY_HANDLED,
  /// The resolution was aborted and we should move on
  RESOLVER_SEND_STATUS_ABORTED,
} resolver_send_status_t;

/**
 * @brief Status code for the frame transmission
 *
 */
typedef enum {
  /// Rule type is a set
  RESOLVER_SET_RULE,
  /// Rule type is a get
  RESOLVER_GET_RULE
} resolver_rule_type_t;

/**
 * @brief Function prototype for getting notified of set rules.
 */
typedef void (*resolver_on_set_rule_registered_t)(attribute_store_type_t);

/**
 * @brief Registers a callback/listening function for the known set rules.
 * This function will be invoked for each known attribute store type that can be set.
 *
 * @param function  The function to call to notify of new set rules
 */
void attribute_resolver_register_set_rule_listener(
  resolver_on_set_rule_registered_t function);

/**
 * @brief Return the get function for a given attribute type
 *
 * @param node_type
 * @returns         NULL if this attribute type cannot be resolved.
 *                  Else the attribute_resolver_function_t that can resolve
 *                  this particular attribute
 */
attribute_resolver_function_t
  attribute_resolver_get_function(attribute_store_type_t node_type);

/**
 * @brief Return the set function for a given attribute type
 *
 * @param node_type
 * @returns         NULL if this attribute type cannot be resolved.
 *                  Else the attribute_resolver_function_t that can resolve
 *                  this particular attribute
 */
attribute_resolver_function_t
  attribute_resolver_set_function(attribute_store_type_t node_type);

/**
 * @brief Returns if the Rule Book can resolve a Set Rule for the attribute type
 *
 * @param node_type The attribute Store node type for which we want to know
 *                  if there is a Set rule registered.
 * @returns true if a set rule is registered
 *          false if no set rule is registered
 */
bool attribute_resolver_has_set_rule(attribute_store_type_t node_type);

/**
 * @brief Returns if the Rule Book can resolve a Get Rule for the attribute type
 *
 * @param node_type The attribute Store node type for which we want to know
 *                  if there is a Get rule registered.
 * @returns true if a Get rule is registered
 *          false if no Get rule is registered
 */
bool attribute_resolver_has_get_rule(attribute_store_type_t node_type);

/**
 * @brief Callback function which must be called when frame tranmission is done
 *
 * This function must be called when a frame sent using attribute_resolver_send
 * is done.
 *
 * @param status    Status of transmission
 * @param node      Attribute node for which the frame was sent.
 * @param transmit_time dureation of the transmission
 * @param rule_type The rule type that was used for resolution.
 */
void on_resolver_send_data_complete(resolver_send_status_t status,
                                    clock_time_t transmit_time,
                                    attribute_store_node_t node,
                                    resolver_rule_type_t rule_type);

#ifdef __cplusplus
}
#endif

#endif  //ATTRIBUTE_RESOLVER_RULE_H
        /** @} end attribute_resolver_rule */