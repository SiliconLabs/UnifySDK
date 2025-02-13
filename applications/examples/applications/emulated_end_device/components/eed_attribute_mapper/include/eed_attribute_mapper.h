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
 * @defgroup eed_attribute_mapper EED Attribute Mapper
 * @ingroup eed_components
 * @brief Initializes and configure the Attribute Mapper for the EED
 *
 * @{
 */

#ifndef EED_ATTRIBUTE_MAPPER_H
#define EED_ATTRIBUTE_MAPPER_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the EED specialization of the attribute mapper.
 *
 * @return SL_STATUS_OK for success, SL_STATUS_FAIL if an error occurred
 */
sl_status_t eed_attribute_mapper_init();

#ifdef __cplusplus
}
#endif

#endif  //EED_ATTRIBUTE_MAPPER_H
/** @} end eed_attribute_mapper */
