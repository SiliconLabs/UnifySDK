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
 * @file attribute_management.h
 * @defgroup attribute_management  ZigPC Attribute Management component
 * @ingroup zigpc_components
 * @brief API and functions for interacting with attributes.
 *
 * @{
 *
**/
#ifndef ATTRIBUTE_MANAGEMENT_H
#define ATTRIBUTE_MANAGEMENT_H

// Shared UIC includes
#include <sl_status.h>

// ZigPC includes
#include <zigpc_common_zigbee.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief configure_attributes_node - API for configuring reports on a given 
 * zigbee node. Only basic reports are supported at this time.
 *
 * @param node - the zigbee node where reports should be configured. expected
 *               to have a valid EUI64 and an endpoint
 *
 * @return the status if the method succeeded and reports were properly 
 * configured
**/
sl_status_t configure_attributes_node(const zigbee_node_t node);

/**
 * @brief configure_attributes_endpoint - API for configuring reports on a
 * a given endpoint. Only basic reports are supported at this time. This 
 * method is required since endpoints can be added on an ad-hoc basis.
 *
 * @param eui64     - the eui64 of the node hosting the endpoint
 * @param endpoint  - the endpoint id whose reports will be configured. Should
 *                    have a valid cluster
 *
 * @return the status if the method succeeded and reports were properly 
 * configured
**/
sl_status_t configure_attributes_endpoint(const zigbee_eui64_t eui64,
                                          const zigbee_endpoint_t endpoint);

#ifdef __cplusplus
}
#endif

#endif  //ATTRIBUTE_MANAGEMENT_H

/** @} end attribute_management */
