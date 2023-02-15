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
 * @file zigpc_binding_mqtt.h
 * @defgroup zigpc_binding_mqtt
 * @ingroup zigpc_components
 * @brief API and functions for interacting with MQTT for bindings.
 *
 * @{
 *
**/

#ifndef ZIGPC_BINDING_MQTT_H
#define ZIGPC_BINDING_MQTT_H

#include "sl_status.h"
#include "zigpc_common_zigbee.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief zigpc_binding_init_mqtt - Initialize
 * all the MQTT topics for the binding cluster
 * 
 * @param eui64 - the address of the node
 * @param endpoint - the endpoint id where the bindings will be initiated
 *
 * @return SL_STATUS_OK if the  MQTT topics could be successfully set up 
**/
sl_status_t zigpc_binding_init_mqtt(
        zigbee_eui64_uint_t eui64,
        zigbee_endpoint_id_t endpoint);

/**
 * @brief zigpc_publish_binding_table - publish the binding
 * table of a node/endpoint combination to MQTT. Fetches the
 * persisted bindings from the datastore and then parses that
 * to MQTT
 *
 * @param eui64 - address of the node
 * @param endpoint - the ID of the endpoint containing the bindings
 *
 * @return SL_STATUS_OK if able to retrieve all information and
 * publish as an MQTT message
**/
sl_status_t zigpc_publish_binding_table(
                zigbee_eui64_uint_t eui64, 
                zigbee_endpoint_id_t endpoint);

/**
 * @brief zigpc_binding_mqtt_update_bindable_clusters - update
 * the clusters which the endpoint can bind to on MQTT
 *
 * @param eui64 - the address of the node
 * @param endpoint - the ID of the endpoint that can be bound
 *
 * @return SL_STATUS_OK if MQTT could be properly updated
**/
sl_status_t 
    zigpc_binding_mqtt_publish_bindable_clusters(
            zigbee_eui64_uint_t eui64,
            zigbee_endpoint_id_t endpoint);

/**
 * @brief zigpc_binding_mqtt_pub_table_full - publish the
 * "BindingTableFull" attribute to MQTT for a given
 * node/endpoint combination
 *
 * @param eui64 - the address of the node
 * @param endpoint - the endpoint id whose binding table status
 * is being published to MQTT
 *
 * @return SL_STATUS_OK if the "BindingTableFull" attribute could be
 * properly updated
**/
sl_status_t 
    zigpc_binding_mqtt_publish_table_full(
        zigbee_eui64_uint_t eui64,
        zigbee_endpoint_id_t endpoint,
        bool is_reported,
        bool table_full_value);

#ifdef __cplusplus
}
#endif
#endif //ZIGPC_BINDING_MQTT_H

