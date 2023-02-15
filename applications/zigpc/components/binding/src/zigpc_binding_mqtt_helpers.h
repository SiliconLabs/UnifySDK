/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @file zigpc_binding_mqtt_helpers.h
 * @defgroup zigpc_binding_mqtt_helpers
 * @ingroup zigpc_components
 * @brief API and functions for helping with MQTT parsing.
 *
 */

#ifndef ZIGPC_BINDING_MQTT_HELPERS_H
#define ZIGPC_BINDING_MQTT_HELPERS_H

#include <string>
#include "sl_status.h"
#include "zigpc_common_zigbee.h"
#include "uic_mqtt.h"

//Information used to interact with the "Binding" cluster
//representation used in the binding component
const std::string BINDING_CLUSTER_NAME = "/Binding" ;
const std::string BINDING_TABLE_TOPIC = "/BindingTable" ;
const std::string BINDING_TABLE_FULL_TOPIC = "/BindingTableFull" ;
const std::string BINDABLE_CLUSTERS_TOPIC = "/BindableClusterList" ;

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief zigpc_binding_mqtt_sub_bind - Subscribe to the MQTT topic
 * for issuing "Binding" commands
 * 
 * @param eui64 - the address of the node
 * @param endpoint - the endpoint id where the bindings will be initiated
 * @param callback - the callback function that should be called when
 * MQTT message for "Binding" is received
 *
 * @return SL_STATUS_OK if the  MQTT callback could be successfully set up 
**/
sl_status_t zigpc_binding_mqtt_sub_bind(
        zigbee_eui64_uint_t eui64,
        zigbee_endpoint_id_t endpoint,
        mqtt_message_callback_t callback);


/**
 * @brief zigpc_binding_mqtt_sub_unbind - Subscribe to the MQTT topic
 * for issuing "Unbinding" commands
 * 
 * @param eui64 - the address of the node
 * @param endpoint - the endpoint id where the bindings will be initiated
 * @param callback - the callback function that should be called when
 * MQTT message for "Unbinding" is received
 *
 * @return SL_STATUS_OK if the  MQTT callback could be successfully set up 
**/
sl_status_t zigpc_binding_mqtt_sub_unbind(
        zigbee_eui64_uint_t eui64,
        zigbee_endpoint_id_t endpoint,
        mqtt_message_callback_t callback);

/**
 * @brief zigpc_binding_mqtt_pub_supported - Publish to the MQTT topic
 * for reporting Supported commands for the "Binding" cluster
 * 
 * @param eui64 - the address of the node
 * @param endpoint - the endpoint id where the bindings will be initiated
 *
 * @return SL_STATUS_OK if the MQTT message could be successfully sent
**/
sl_status_t zigpc_binding_mqtt_pub_supported(
        zigbee_eui64_uint_t eui64,
        zigbee_endpoint_id_t endpoint);


/**
 * @brief zigpc_binding_mqtt_pub_generated - Publish to the MQTT topic
 * for reporting Generated commands for the "Binding" cluster
 * 
 * @param eui64 - the address of the node
 * @param endpoint - the endpoint id where the bindings will be initiated
 *
 * @return SL_STATUS_OK if the MQTT message could be successfully sent
**/
sl_status_t zigpc_binding_mqtt_pub_generated(
        zigbee_eui64_uint_t eui64,
        zigbee_endpoint_id_t endpoint);

#ifdef __cplusplus
}
#endif
#endif //ZIGPC_BINDING_MQTT_HELPERS_H
