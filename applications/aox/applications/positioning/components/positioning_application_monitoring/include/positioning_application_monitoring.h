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
 * @defgroup positioning_application_monitoring Positioning Application Monitoring
 * @ingroup positioning_components
 * @brief Initializes the Positioning Application Monitoring
 *
 * @{
 */

#ifndef POSITIONING_APPLICATION_MONITORING_H
#define POSITIONING_APPLICATION_MONITORING_H

#include "sl_status.h"

// Name of the positioning application used for Application Monitoring
#define POSITIONING_APPLICATION_NAME "Positioning Application"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the ApplicationMonitoring resources for the positioning
 * Application.
 *
 * @return SL_STATUS_OK on success, any other value in case of error.
 */
sl_status_t positioning_application_monitoring_init();

#ifdef __cplusplus
}
#endif

#endif  //POSITIONING_APPLICATION_MONITORING_H
/** @} end positioning_application_monitoring */
