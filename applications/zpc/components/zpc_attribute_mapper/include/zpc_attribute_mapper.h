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
 * @defgroup zpc_attribute_mapper ZPC Attribute Mapper
 * @ingroup zpc_components
 * @brief Initializes and configure the Attribute Mapper for the ZPC
 *
 * @{
 */

#ifndef ZPC_ATTRIBUTE_MAPPER_H
#define ZPC_ATTRIBUTE_MAPPER_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the ZPC specialization of the attribute mapper.
 *
 * @return SL_STATUS_OK for success, SL_STATUS_FAIL if an error occurred
 */
sl_status_t zpc_attribute_mapper_init();

#ifdef __cplusplus
}
#endif

#endif  //ZPC_ATTRIBUTE_MAPPER_H
/** @} end zpc_attribute_mapper */
