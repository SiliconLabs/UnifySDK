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
 * @file zwave_api_demo_callbacks.h
 * @ingroup ZWAVE_API_DEMO
 * @brief List of callbacks for incoming messages from the Z-Wave module in
 * the Z-Wave API demo application
 *
 * The Z-Wave API demo application demonstrate basic use of the Z-Wave API.
 * A number of callback functions are required to make use of the Z-Wave API for
 * when the Z-Wave module sends some messages.
 *
 * @{
 */

#ifndef ZWAVE_API_DEMO_CALLBACKS_H
#define ZWAVE_API_DEMO_CALLBACKS_H

#define MAXIMUM_MESSAGE_SIZE 1000
///< Maximum size for a string/message being printed to the user
#define INPUT_BUFFER_SIZE 255  ///< Maximum buffer size for command line inputs

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief application_command_handler(_bridge) function for @ref zwapi_callbacks.
 * Incoming frames from the Z-Wave nodes (e.g. Basic Report) will be
 * dispatched to this function.
 */
void zwapi_demo_application_handler(uint8_t rx_status,
                                    zwave_node_id_t destination_node_id,
                                    zwave_node_id_t source_node_id,
                                    const uint8_t *zwave_command,
                                    uint8_t zwave_command_length,
                                    int8_t rssi_value);

/**
 * @brief application_controller_update function for @ref zwapi_callbacks.
 * Incoming NIF from the Z-Wave nodes will be dispatched to this
 * function.
 */
void zwapi_demo_application_controller_update(uint8_t status,
                                              zwave_node_id_t node_id,
                                              const uint8_t *zwave_nif,
                                              uint8_t zwave_nif_length,
                                              zwave_home_id_t nwi_home_id);

/**
 * @brief functions handling when the z-wave api is ready to use. This
 * is called after soft reset. See @ref zwapi_callbacks
 */
void zwapi_demo_zwave_api_started(const uint8_t *buffer, uint8_t buffer_length);

/**
 * @brief callback function for @ref zwapi_send_node_information()
 */
void zwapi_demo_send_nif_callback(uint8_t tx_status);

/**
 * @brief callback function for @ref zwapi_add_node_to_network()
 */
void zwapi_demo_node_add_callback(LEARN_INFO *new_node_information);

/**
 * @brief callback function for @ref zwapi_remove_node_from_network()
 */
void zwapi_demo_node_remove_callback(LEARN_INFO *new_node_information);

/**
 * @brief callback function for @ref zwapi_remove_failed_node()
 */
void zwapi_demo_node_remove_failed_callback(uint8_t tx_status);

/**
 * @brief callback function for @ref zwapi_send_data()
 */
void zwapi_demo_send_data_callback(uint8_t tx_status,
                                   zwapi_tx_report_t *tx_status_report);

/**
 * @brief callback function for @ref zwapi_request_node_info()
 */
void zwapi_demo_request_nif_callback(uint8_t tx_status,
                                     zwapi_tx_report_t *tx_status_report);

/**
 * @brief callback function for @ref zwapi_set_learn_mode()
 */
void zwapi_demo_learn_mode_callback(LEARN_INFO *learn_mode_info);

/**
 * @brief callback function for @ref zwapi_set_default()
 */
void zwapi_demo_set_default_callback(void);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_API_DEMO_CALLBACKS_H
        /** @} end ZWAVE_API_DEMO */
