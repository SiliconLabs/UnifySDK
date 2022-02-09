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
 * @defgroup zpc_attribute_resolver_internals ZPC Attribute Resolver Internals
 * @ingroup zpc_attribute_resolver
 * @brief Internal functions for the @ref zpc_attribute_resolver.
 */

/**
 * @defgroup attribute_resolver_send ZPC Attribute Resolver send
 * @ingroup zpc_attribute_resolver_internals
 * @brief Send frames when instructed by the @ref attribute_resolver
 *
 * @{
 */

#ifndef ATTRIBUTE_RESOLVER_SEND_H
#define ATTRIBUTE_RESOLVER_SEND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "attribute_store.h"
#include "zwave_unid.h"

/**
 * @brief function that the ZPC attribute resolver uses send PAN frames
 *
 * The function on_resolver_send_data_complete must be called when the frame
 * transmission is complete
 *
 * @param node            Attribute node for which the frame has to be sent
 * @param frame_data      Genereted data to send
 * @param frame_data_len  Length of generated data
 * @param is_set          Is a Set-frame
 * @returns sl_status_t SL_STATUS_OK if the frame is queued for transmission.
 *                      Any other code in case of error.
 */
sl_status_t attribute_resolver_send(attribute_store_node_t node,
                                    const uint8_t *frame_data,
                                    uint16_t frame_data_len,
                                    bool is_set);

/**
 * @brief Initialization function for the attribute_resolver_send subcomponent
 *
 * Allocates a node_under_resolution map to prepare handling simultaneous resolutions
 */
void attribute_resolver_send_init();

#ifdef __cplusplus
}
#endif

#endif  //ATTRIBUTE_RESOLVER_SEND_H
/** @} end attribute_resolver_send */
