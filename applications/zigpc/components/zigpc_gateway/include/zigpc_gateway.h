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

/**
 * @file zigpc_gateway.h
 * @defgroup zigpc_gateway ZigPC Gateway Component
 * @ingroup zigpc_components
 * @brief Public command API for zigpc_gateway
 *
 * @{
 */
#ifndef ZIGPC_GATEWAY_H
#define ZIGPC_GATEWAY_H

#include "zigpc_common_zigbee.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Request to  initialize the Zigbee Network. The gateway will establish
 * the Gateway device as a Zigbee Coordinator and Trust Center. On success, the
 * "on_network_initialized" event will be set to components listening on
 * zigpc_gateway notifications.
 *
 * @return sl_status_t          SL_STATUS_OK on request accepted by ZigPC
 * Gateway, or error in reading and preparing data to be sent to ZigPC Gateway.
 */
sl_status_t zigpc_gateway_network_init(void);

/**
 * @brief Request to enable/disable permitting Zigbee devices to join the
 * network.
 *
 * @return sl_status_t          SL_STATUS_OK on request accepted by ZigPC
 * Gateway, or error in reading and preparing data to be sent to ZigPC Gateway.
 */
sl_status_t zigpc_gateway_network_permit_joins(bool enable);

/**
 *@brief Request to send a ZCL frame to an end device.
 *
 * @param eui64                 End device EUI64 in big-endian format.
 * @param endpoint_id           End device endpoint identifier.
 * @param cluster_id            End device ZCL cluster identifier.
 * @param frame                 ZCL Frame to send.
 * @return sl_status_t          SL_STATUS_OK on request accepted by ZigPC
 * Gateway, or error in reading and preparing data to be sent to ZigPC Gateway.
**/
sl_status_t
  zigpc_gateway_send_zcl_command_frame(const zigbee_eui64_t eui64,
                                       zigbee_endpoint_id_t endpoint_id,
                                       zcl_cluster_id_t cluster_id,
                                       const zcl_frame_t *frame);

/**
 *@brief Request to send a ZCL frame to a multicast group.
 *
 * @param multicast_id          Multicast identifier.
 * @param frame                 ZCL Frame to send.
 * @return sl_status_t          SL_STATUS_OK on request accepted by ZigPC
 * Gateway, or error in reading and preparing data to be sent to ZigPC Gateway.
**/
sl_status_t
  zigpc_gateway_send_zcl_frame_multicast(zigbee_multicast_id_t multicast_id,
                                         zcl_cluster_id_t cluster_id,
                                         const zcl_frame_t *frame);

/**
 *@brief Request to send a ZCL Configure Reporting command to an end device.
 *
 * @param eui64                 End device EUI64 in big-endian format.
 * @param endpoint_id           End device endpoint identifier.
 * @param cluster_id            End device ZCL cluster identifier.
 * @param report_record         ZCL report record to send.
 * @param record_size           ZCL report record size.
 * @return sl_status_t          SL_STATUS_OK on request accepted by ZigPC
 * Gateway, or error in reading and preparing data to be sent to ZigPC Gateway.
**/
sl_status_t zigpc_gateway_configure_reports(const zigbee_eui64_t eui64,
                                            uint8_t endpoint_id,
                                            zcl_cluster_id_t cluster_id,
                                            const uint8_t *report_record,
                                            unsigned int record_size);

/**
 *@brief Request to send a ZCL Configure Reporting command to an end device.
 *
 * @param eui64                 End device EUI64 in big-endian format.
 * @param endpoint_id           End device endpoint identifier.
 * @param cluster_id            End device ZCL cluster identifier.
 * @return sl_status_t          SL_STATUS_OK on request accepted by ZigPC
 * Gateway, or error in reading and preparing data to be sent to ZigPC Gateway.
**/
sl_status_t zigpc_gateway_request_binding(const zigbee_eui64_t eui64,
                                          uint8_t endpoint_id,
                                          zcl_cluster_id_t cluster_id);

/**
 *@brief Request to send a ZCL Configure Reporting command to an end device.
 *
 * @param eui64                 End device EUI64 in big-endian format.
 * @param endpoint_id           End device endpoint identifier.
 * @param cluster_id            End device ZCL cluster identifier.
 * @param report_record         ZCL report record to send.
 * @param record_size           ZCL report record size.
 * @return sl_status_t          SL_STATUS_OK on request accepted by ZigPC
 * Gateway, or error in reading and preparing data to be sent to ZigPC Gateway.
**/
sl_status_t
  zigpc_gateway_request_binding_endpoint(const zigbee_eui64_t eui64,
                                         const zigbee_endpoint_t endpoint);
/**
 * @brief Request to add a node to the Zigbee Network using the Z3 Install Code
 * Method.
 *
 * @param node_eui64            End device EUI64 in big-endian format
 * @param install_code          End device install code
 * @param install_code_length   install code length
 * @return sl_status_t          SL_STATUS_OK on request accepted by ZigPC
 * Gateway, or error in reading and preparing data to be sent to ZigPC Gateway.
 */
sl_status_t
  zigpc_gateway_add_node_install_code(const zigbee_eui64_t node_eui64,
                                      const zigbee_install_code_t install_code,
                                      const uint8_t install_code_length);

/**
 * @brief Request to remove node on the Zigbee Network. If the node is not on
 * the Zigbee network, no device remove success events will be sent to
 * listening components.
 *
 * NOTE: Callers should use a timeout to respond to failure in removing
 * a device if no notifications are sent within that timeout.
 *
 * @param eui64                 End device EUI64 in big-endian format
 * @return sl_status_t          SL_STATUS_OK on request accepted by ZigPC
 * Gateway, or error in reading and preparing data to be sent to ZigPC Gateway.
 */
sl_status_t zigpc_gateway_remove_node(const zigbee_eui64_t eui64);

/**
 * @brief Request to interview node on the Zigbee Network. If the node is not on
 * the Zigbee network, no device or endpoint discovery notifications will be
 * sent.
 *
 * NOTE: Callers should use a timeout to respond to failure in interviewing a
 * device if no notifications are sent within that timeout.
 *
 * @param eui64                 End device EUI64 in big-endian format
 * @return sl_status_t          SL_STATUS_OK on request accepted by ZigPC
 * Gateway, or error in reading and preparing data to be sent to ZigPC Gateway.
 */
sl_status_t zigpc_gateway_interview_node(const zigbee_eui64_t eui64);

/**
 * @brief zigpc_gateway_add_ota_image - add an OTA image to the ota-server plugin
 * 
 * @param filename - the filepath of the requested OTA image
 *
 * @return SL_STATUS_SUCCESS if able to add the filename properly
 **/ 
sl_status_t zigpc_gateway_add_ota_image(
                const char* filename,
                unsigned int filename_size);

/**
 * @brief Send command to print Z3Gateway info.
 *
 */
void zigpc_gateway_command_print_info(void);

/**
 * @brief Send command to print Ember NWK key.
 *
 */
void zigpc_gateway_command_print_nwk_key(void);

/**
 * @brief Get the counters list array of values
 * 
 * @param buffer the array to be filled
 * @param size number of index in the array
 * @return sl_status SL_STATUS_OK on success or error on fail
 */
sl_status_t zigpc_gateway_get_counters_list(uint16_t *buffer, size_t count);

/**
 * @brief Get the label of the counters entry based on the offset passed in.
 * 
 * @param offset counter entry offset to get label for.
 * @return const char* counter entry label if valid, "NOT FOUND" otherwise.
 */
const char *zigpc_gateway_get_counters_entry_label(size_t offset);

/**
 * @brief Get the counters capacity
 * 
 * @return size_t capacity
 */
size_t zigpc_gateway_get_counters_capacity();

/**
 * @brief clear the counter plugin tokens
 * 
 */
void zigpc_gateway_clear_counters();

#ifdef __cplusplus
}
#endif

#endif  //ZIGPC_GATEWAY_H

/** @} end zigpc_gateway */
