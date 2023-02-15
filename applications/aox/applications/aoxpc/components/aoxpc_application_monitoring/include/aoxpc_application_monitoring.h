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
 * @defgroup aoxpc_application_monitoring AOXPC Application Monitoring
 * @ingroup aoxpc_components
 * @brief Initializes the AOXPC Application Monitoring
 *
 * @{
 */

#ifndef AOXPC_APPLICATION_MONITORING_H
#define AOXPC_APPLICATION_MONITORING_H

#include "sl_status.h"

// Name of the AOXPC application used for Application Monitoring
#define AOXPC_APPLICATION_NAME "AoXPC (Angle-of-X Protocol Controller)"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the ApplicationMonitoring resources for the AOXPC.
 *
 * @return SL_STATUS_OK on success, any other value in case of error.
 */
sl_status_t aoxpc_application_monitoring_init();

#ifdef __cplusplus
}
#endif

#endif  //AOXPC_APPLICATION_MONITORING_H
/** @} end aoxpc_application_monitoring */
