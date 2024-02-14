/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup unify_dotdot_attribute_store_descriptor UCL device type
 * @ingroup ucl_mqtt
 * @brief Handles publishing device type list to `ucl/by-unid/<node>/DeviceTypeList`.
 *
 * This component handles publishing the `ucl/by-unid/<node>/DeviceTypeList` topic in 
 * MQTT. It register callbacks in the \ref attribute_store for DeviceTypeList attribute
 * and acts on these callbacks, when the list changes, and publish the changes to MQTT.

 */

#ifndef UNIFY_DOTDOT_ATTRIBUTE_STORE_DESCRIPTOR_H
#define UNIFY_DOTDOT_ATTRIBUTE_STORE_DESCRIPTOR_H

#include "uic_enum.h"
#include "attribute_store.h"

#ifdef __cplusplus
extern "C" {
#endif

sl_status_t unify_dotdot_attribute_store_descriptor_init();

#ifdef __cplusplus
}
#endif

#endif  //UNIFY_DOTDOT_ATTRIBUTE_STORE_NODE_STATE_H
/** @} end unify_dotdot_attribute_store_node_state */


