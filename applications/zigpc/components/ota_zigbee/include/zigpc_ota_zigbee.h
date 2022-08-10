/*******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

/**
 * @file ota_zigbee.h
 * @defgroup ota_zigbee ZigPC OTA functions
 * @ingroup zigpc_components
 * @brief This header contains the definitions and functionality associated with
 * over-the-air upgrades setup. This is the intended public interface for the
 * entire component. All definitions and functions here are required for
 * manipulating or configuring OTA Updates
 *
 * @{
 */

#ifndef OTA_ZIGBEE_H
#define OTA_ZIGBEE_H

// Shared Unify includes
#include <sl_status.h>

// ZigPC includes
#include <zigpc_common_zigbee.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief zigpc_ota_init
 * Initialize the OTA Component
 *
 * @return SL_STATUS_OK if able to successfully initialize the component
 */
sl_status_t zigpc_ota_init();

/*
 * @brief zigpc_ota_configure_node
 * Configure OTA Updates for a node
 *
 * @param eui64 - the eui64 of the node to initialize
 *
 * @return SL_STATUS_OK if successfully able to configure
 * OTA Updates for a node
 */
sl_status_t zigpc_ota_configure_node(const zigbee_eui64_t eui64);
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif  //OTA_ZIGBEE_H

/** @} end ota_zigbee */
