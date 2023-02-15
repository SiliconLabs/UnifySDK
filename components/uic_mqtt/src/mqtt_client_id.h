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
 * @defgroup mqtt_client_id MQTT Client ID
 * @ingroup uic_mqtt
 * @brief Generates and provide a Client ID for the application/machine
 *
 * This component provide an MQTT Client ID for each application that will
 * be deterministic (i.e. identical on reboot) and unique to each application
 * /machine.
 *
 * @{
 */

#ifndef MQTT_CLIENT_ID_H
#define MQTT_CLIENT_ID_H

// Key used to store/fetch the MQTT Client ID used by the application.
#define DATASTORE_MQTT_CLIENT_ID_KEY "mqtt_client_client_id"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Reads the configuration and determines which MQTT Client ID we should use
 *
 * @return const char* C-String for the Client ID to use.
 */
const char *mqtt_client_id_get_from_config();

/**
 * @brief Stores the MQTT Client ID we used in the datastore, if any datastore
 * is active with the current application
 *
 * This function will emit an error message if a different MQTT Client ID
 * was stored in the database before.
 *
 * @param client_id_in_use Current MQTT Client ID used for the broker.
 */
void mqtt_client_store_client_id_to_datastore(const char *client_id_in_use);

#ifdef __cplusplus
}
#endif

#endif  //MQTT_CLIENT_ID_H
/** @} end mqtt_client_id */
