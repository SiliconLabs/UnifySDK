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
 * @defgroup eed_application_monitoring EED Application Monitoring
 * @ingroup eed_components
 * @brief Initializes the EED Application Monitoring
 *
 * @{
 */

#ifndef EED_APPLICATION_MONITORING_H
#define EED_APPLICATION_MONITORING_H

#include "sl_status.h"

// Name of the EED application used for Application Monitoring
#define EED_APPLICATION_NAME "EED (Emulated End Device)"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the ApplicationMonitoring resources for the EED.
 *
 * @return SL_STATUS_OK on success, any other value in case of error.
 */
sl_status_t eed_application_monitoring_init();

#ifdef __cplusplus
}
#endif

#endif  //EED_APPLICATION_MONITORING_H
/** @} end eed_application_monitoring */
