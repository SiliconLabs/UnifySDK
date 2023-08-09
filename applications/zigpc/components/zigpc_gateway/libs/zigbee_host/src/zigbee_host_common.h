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

#ifndef ZIGBEE_HOST_COMMON_H
#define ZIGBEE_HOST_COMMON_H

#include <stddef.h>
#include <string.h>

#include "sl_service_function.h"
#include "app/framework/include/af-types.h"
#include "app/framework/plugin/debug-print/sl_zigbee_debug_print.h"
#include "zigbee_host.h"

/**
 * @brief Size of buffer used to send ZCL frames
 */
#define ZCL_BUFFER_SIZE 255

#ifndef UNUSED_VAR

/**
 * @brief Useful macro for avoiding compiler warnings related to unused
 * function arguments or unused variables.
 */
#define UNUSED_VAR(x) (void)(x)
#endif

#define ZIGBEE_HOST_CALLBACK_EXISTS(callback_struct, callback_event) \
  ((callback_struct != NULL) && (callback_struct->callback_event != NULL))

#define LOG_FMTSTR_EUI64_NODEID_RES_FAIL \
  "Error: Failed to resolve device %s for %s\n"
#define LOG_FMTSTR_SOURCE_EUI64_RES_FAIL \
  "Error: Failed to source EUI64 %s event\n"
#define LOG_FMTSTR_UNREGISTERED_CALLBACK \
  "Error: Ignoring unregistered callback for %s event\n"

#define appDebugPrint(...) sl_zigbee_app_debug_print(__VA_ARGS__)
#define appDebugPrintBuffer(buffer, length, with_space) \
  sl_zigbee_app_debug_print_buffer(buffer, length, with_space)

/**
 * @brief Internal state
 *
 */
struct zigbeeHostState {
  struct zigbeeHostCallbacks *callbacks;
};

extern struct zigbeeHostState z3gwState;

/**
  * @brief Cluster discovery handler for endpoint requested.
  * For each match, the callback is fired with all the resulting matches from
  * that source.  If the discovery was unicast to a specific device, then
  * the callback will only be fired once with either MATCH_FOUND or COMPLETE
  * (no matches found).  If the discovery is broadcast then multiple callbacks
  * may be fired with ::EMBER_AF_BROADCAST_SERVICE_DISCOVERY_RESPONSE_RECEIVED.
  * After a couple seconds the callback will then be fired with
  * ::EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE or
  * ::EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE_WITH_EMPTY_RESPONSE as the result.
  */
void zigbeeHostZdoActiveEndpointsResponseCallback(
  const EmberAfServiceDiscoveryResult *result);

/**
  * @brief Endpoint discovery handler for a device requested.
  * For each match, the callback is fired with all the resulting matches from
  * that source.  If the discovery was unicast to a specific device, then
  * the callback will only be fired once with either MATCH_FOUND or COMPLETE
  * (no matches found).  If the discovery is broadcast then multiple callbacks
  * may be fired with ::EMBER_AF_BROADCAST_SERVICE_DISCOVERY_RESPONSE_RECEIVED.
  * After a couple seconds the callback will then be fired with
  * ::EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE or
  * ::EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE_WITH_EMPTY_RESPONSE as the result.
  */
void zigbeeHostZdoSimpleDescriptorResponseCallback(
  const EmberAfServiceDiscoveryResult *result);

/** @brief emAfPluginGatewayInterfaceTrustCenterJoinHandler
 *
 * This callback is called to inform when a device has joined the Zigbee
 * Trust Center.
 *
 * @param eui64 Ver.: always
 * @param nodeId Ver.: always
 * @param decision Ver.: Always
 */
void emAfPluginGatewayInterfaceTrustCenterJoinHandler(
  const EmberEUI64 eui64, EmberNodeId nodeId, EmberJoinDecision decision);

/**
 * @brief Device Table plugin callback when a device has completed leaving
 * the network.
 *
 *@param eui64:  EUI64 of the device that left.
 */
void emAfPluginGatewayInterfaceTrustCenterLeaveHandler(const EmberEUI64 eui64);

EmberAfStatus
  emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                sl_service_function_context_t *context);
/**
 * @brief Register the cluster callbacks for a given cluster list
 *
 * @param cluster_list: the list of supported clusters as an array of ID's
 * @param cluster_list_size: the size of the supported cluster list
 *
 * @return EMBER_SUCCESS if able to properly register the service callbacks for
 * the given cluster
 */
EmberStatus zigbeeHostRegisterClusters(
              const uint16_t *cluster_list,
              unsigned int cluster_list_size );

#endif  // ZIGBEE_HOST_COMMON_H
