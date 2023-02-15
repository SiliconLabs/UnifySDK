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
 * @defgroup unify_application_monitoring
 * @ingroup unify_components
 * @brief Helper components that assists with ApplicationMonitoring over MQTT.
 *
 * @{
 */

#ifndef UNIFY_APPLICATION_MONITORING_H
#define UNIFY_APPLICATION_MONITORING_H

#include <stdbool.h>
#include "sl_status.h"

// Constants
#define APPLICATION_MONITORING_CLUSTER_REVISION 1

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Activates the ApplicationMonitoring functionality.
 *
 * After invoking this function, attributes and supported commands will be
 * published at the ucl/by-mqtt-client/<client-id>/ApplicationMonitoring cluster.
 *
 * @returns SL_STATUS_OK in case of success, any other value in case of error
 */
sl_status_t unify_application_monitoring_init();

/**
 * @brief Configure the name of the application currently using the
 * ApplicationMonitoring cluster.
 *
 * @param application_name String to set for the application name.
 */
void unify_application_monitoring_set_application_name(
  const char *application_name);

/**
 * @brief Configure the version of the application currently using the
 * ApplicationMonitoring cluster.
 *
 * @param application_version String to set for the application name.
 */
void unify_application_monitoring_set_application_version(
  const char *application_version);

/**
 * @brief Cleans up all Application monitoring topics and de-activate
 * the application monitoring functionality
 */
void unify_application_monitoring_teardown();

#ifdef __cplusplus
}
#endif

#endif  //UNIFY_APPLICATION_MONITORING_H
/** @} end unify_application_monitoring */
