/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @file zwapi_init.h
 * @brief Z-Wave API initialization functions.
 *
 */

#ifndef ZWAPI_INIT_H
#define ZWAPI_INIT_H

#include <stdint.h>
#include <stdbool.h>
#include "sl_status.h"
#include "zwapi_protocol_basis.h"
#include "zwave_utils.h"
#include "zwave_rf_region.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup ZWAPI_INIT Initialization
 * @ingroup ZWAPI
 * @brief Initialization of the Z-Wave API
 *
 * This module contains functions to both initialize and shut down the Z-Wave
 * API and query the Z-Wave module about supported functions.
 * @{
 */

/// @name zwapi_library_type_t
/// One of these defines are returned when requesting library type with
/// zwapi_get_zw_library_type() / zwapi_get_library_type()
///@{
typedef enum {
  ZWAVE_LIBRARY_TYPE_CONTROLLER_STATIC
  = 0x01,                                ///< @zgw_name ZW_LIB_CONTROLLER_STATIC
  ZWAVE_LIBRARY_TYPE_CONTROLLER = 0x02,  ///< @zgw_name ZW_LIB_CONTROLLER
  ZWAVE_LIBRARY_TYPE_END_NODE_ENHANCED
  = 0x03,                               ///< @zgw_name ZW_LIB_SLAVE_ENHANCED
  ZWAVE_LIBRARY_TYPE_END_NODE  = 0x04,  ///< @zgw_name ZW_LIB_SLAVE
  ZWAVE_LIBRARY_TYPE_INSTALLER = 0x05,  ///< @zgw_name ZW_LIB_INSTALLER
  ZWAVE_LIBRARY_TYPE_END_NODE_ROUTING
  = 0x06,  ///< @zgw_name ZW_LIB_SLAVE_ROUTING
  ZWAVE_LIBRARY_TYPE_CONTROLLER_BRIDGE
  = 0x07,                              ///< @zgw_name ZW_LIB_CONTROLLER_BRIDGE
  ZWAVE_LIBRARY_TYPE_DUT      = 0x08,  ///< @zgw_name ZW_LIB_DUT
  ZWAVE_LIBRARY_TYPE_AVREMOTE = 0x0A,  ///< @zgw_name ZW_LIB_AVREMOTE
  ZWAVE_LIBRARY_TYPE_AVDEVICE = 0x0B   ///< @zgw_name ZW_LIB_AVDEVICE
} zwapi_library_type_t;
///@}

/// @name zwave_lr_channel_t
/// List of available Z-Wave Long Range Channels. Refer to
/// zwapi_get_zwave_lr_channel() and zwapi_set_zwave_lr_channel
///@{
typedef enum {
  ZWAVE_LONG_RANGE_CHANNEL_NOT_SUPPORTED
  = 0x00,                             ///< Z-Wave LR is not supported
  ZWAVE_LONG_RANGE_CHANNEL_A = 0x01,  ///< Z-Wave Long Range Channel A
  ZWAVE_LONG_RANGE_CHANNEL_B = 0x02   ///< Z-Wave Long Range Channel B
} zwave_lr_channel_t;
///@}

#define ZWAPI_MAXIMUM_BITMASK_LENGTH (255 / 8)
/// @name Flags used in FUNC_ID_SERIAL_API_GET_INIT_DATA functionality
///@{
#define GET_INIT_DATA_FLAG_END_NODE_API  0x01
#define GET_INIT_DATA_FLAG_TIMER_SUPPORT 0x02
/// @deprecated Use GET_INIT_DATA_FLAG_SECONDARY_CTRL instead
#define GET_INIT_DATA_FLAG_CONTROLLER_STATUS 0x04
#define GET_INIT_DATA_FLAG_SECONDARY_CTRL    0x04
#define GET_INIT_DATA_FLAG_IS_SUC            0x08
///@}

/// @name zwapi_chip_data_t
/// Recommended structure for applications to hold initialization data about the
/// Z-Wave module.
///@{
typedef struct {
  uint8_t chip_type;      ///< Represents the chip hardware.
  uint8_t chip_revision;  ///< Represents the chip hardware revision
  ///< Major version number for the Z-Wave module API software
  uint8_t version_major;
  ///< Minor version number for the Z-Wave module API software.
  uint8_t version_minor;
  ///< Library type of the software running the Z-Wave API
  zwapi_library_type_t library_type;
  ///< Z-Wave Manufacturer ID of the Z-Wave module. Note: This is for
  ///< information only, the Z-Wave Controller is expected to use its own values.
  uint16_t manufacturer_id;
  ///< Z-Wave Product Type, defined by the manufacturer themselves.
  ///< Note: This is for information only, the Z-Wave Controller
  ///< is expected to use its own values.
  uint16_t product_type;
  ///< Z-Wave Product ID, defined by the manufacturer themselves. Note: This is
  ///< for information only, the Z-Wave Controller is expected to use its own values.
  uint16_t product_id;
  ///< Bitmask of supported FUNC_ID for the Z-Wave API.
  uint8_t supported_bitmask[ZWAPI_MAXIMUM_BITMASK_LENGTH];
  ///< Bitmask of supported subcommands for the setup function.
  uint8_t supported_setup_bitmask;
  ///< Current RF Region setting
  zwave_rf_region_t rf_region;
  ///< Z-Wave module capabilities
  uint8_t capability_flag;
  ///< List of existing nodes in the network
  zwave_nodemask_t node_list;
} zwapi_chip_data_t;
///@}

// Size of a git commit hash
#define GIT_COMMIT_HASH_SIZE 16

/* Protocol version structure used in zwapi_get_protocol_version() */
typedef struct _zwapi_protocol_version_information_t_ {
  uint8_t type;  // Type is a strange field that is not really used
  uint8_t major_version;
  uint8_t minor_version;
  uint8_t revision_version;
  uint16_t build_number;
  uint8_t git_commit[GIT_COMMIT_HASH_SIZE];
} zwapi_protocol_version_information_t;

/**
 * @brief Application command handler function type definition
 *
 * @param rx_status received frame status flags:
 *
 *   |Flag Name                            |Flag Mask|Flag description                                                                 |
 *   |-------------------------------------|---------|---------------------------------------------------------------------------------|
 *   | {@ref RECEIVE_STATUS_ROUTED_BUSY}   |xxxxxxx1 |A response route is locked by the application                                    |
 *   | {@ref RECEIVE_STATUS_LOW_POWER}     |xxxxxx1x |Received at low output power level                                               |
 *   | {@ref RECEIVE_STATUS_TYPE_SINGLE}   |xxxx00xx |Received a single cast frame                                                     |
 *   | {@ref RECEIVE_STATUS_TYPE_BROAD}    |xxxx01xx |Received a broadcast frame                                                       |
 *   | {@ref RECEIVE_STATUS_TYPE_MULTI}    |xxxx10xx |Received a multicast frame                                                       |
 *   | {@ref RECEIVE_STATUS_FOREIGN_FRAME} |x1xxxxxx |The received frame is not addressed to this node (Only valid in promiscuous mode)|
 *
 * @param destination_node_id command destination Node ID. Only valid in promiscuous mode
 * and in singlecast frames. Else it will be set to 0 and indicates that it was
 * sent to the Z-Wave module current NodeID
 * @param source_node_id command sender Node ID
 * @param received_frame Payload of the received frame (The command class identifier is
 * is the very first byte)
 * @param received_frame_length is the length of the received command class frame.
 * @param rssi_value is the RSSI measurement for the frame that was received.
 *
 * @serial_rx{REQ | 0x04 | rxStatus | sourceNode | cmdLength | pCmd[ ] | rxRSSIVal }
 *
 * When a foreign frame is received in promiscuous mode:
 * @serial_rx{REQ | 0xD1 | rxStatus | sourceNode | cmdLength | pCmd[ ] | destNode}
 * The destNode parameter is only valid for singlecast frames.
 *
 * The Z-Wave API will call the @ref application_command_handler_function function
 * registered with zwapi_init() when an application command or request has been
 * received from another node. The receive buffer is released when returning
 * from this function. The type of frame used by the request can be determined
 * (single cast, multicast, or broadcast frame). This is used to avoid flooding
 * the network by responding on a multicast or broadcast. In order not to
 * disrupt the radio communication and the protocol, no application function
 * must execute code for more than 5 ms without returning.
 */
typedef void (*application_command_handler_function)(
  uint8_t rx_status,
  zwave_node_id_t destination_node_id,
  zwave_node_id_t source_node_id,
  const uint8_t *received_frame,
  uint8_t received_frame_length,
  int8_t rssi_value);

/**
 * @brief Application controller update function type definition
 *
 * @param status The status of the update process, value could be one of the
 * following:
 * - UPDATE_STATE_ADD_DONE - A new node has been added to the network.
 * - @ref UPDATE_STATE_DELETE_DONE - A node has been deleted from the network.
 * - @ref UPDATE_STATE_NODE_INFO_RECEIVED - A node has sent its node info either
 *                                        unsolicited or as a response to a
 *                                        zwapi_request_node_info() call.
 * - @ref UPDATE_STATE_SUC_ID - The SIS node Id was updated.
 * @param node_id  is the updated node's node ID (1..232).
 * @param zwave_nif is pointer of the updated node's node info.
 * @param zwave_nif_length is the length of the pCmd parameter.
 * @param nwi_home_id   is the NWI HomeID of the node issuing the NIF / INIF /
 *                      SmartStart Prime Command
 *
 * A controller application MAY use the information provided by
 * @ref application_controller_update_function to update local data structures.
 *
 * The Z-Wave protocol MUST notify a controller application by calling
 * @ref application_controller_update_function when a new node has been added or
 * deleted from the controller through the network management features.
 *
 * The Z-Wave protocol MUST call @ref application_controller_update_function in
 * response to @ref zwapi_request_node_info() being called by the controller
 * application. The Z-Wave protocol MAY notify a controller application by
 * calling @ref application_controller_update_function when a Node Information Frame
 * has been received. The Z-Wave protocol MAY refrain from calling the function
 * if the protocol is currently expecting a Node Information frame.
 *
 * @ref application_controller_update_function MUST be called in a controller node
 * operating as SIS each time a node is added or deleted by the primary
 * controller.
 * @ref application_controller_update_function MUST be called in a controller node
 * operating as SIS each time a node is added/deleted by an inclusion
 * controller.
 *
 * The Z Wave protocol may notify a controller application by calling
 * @ref application_controller_update_function when a Node Information Frame has been
 * received. The Z-Wave protocol may not call the function if the
 * protocol is currently expecting node information.
 *
 * A controller application may send a zwapi_request_network_update() command to a
 * SIS or SIS node. In response, the SIS MUST return update information for each
 * node change since the last update handled by the requesting controller node.
 * The application of the requesting controller node may receive multiple calls
 * to @ref application_controller_update_function in response to
 * zwapi_request_network_update().
 *
 * The Z Wave protocol most not call @ref application_controller_update_function in a
 * controller node acting as primary controller or inclusion controller when a
 * node is added or deleted.
 *
 * Any controller application MUST implement this function.
 *
 * \serial_rx{REQ | 0x49 | bStatus | bNodeID | bLen | basic | generic | specific | commandclasses[ ]}
 *
 * application_controller_update_function via the Z-Wave API also have the possibility
 * for receiving the status UPDATE_STATE_NODE_INFO_REQ_FAILED, which means that
 * a node did not acknowledge a zwapi_request_node_info() call.
 */
typedef void (*application_controller_update_function)(
  uint8_t status,
  zwave_node_id_t node_id,
  const uint8_t *zwave_nif,
  uint8_t zwave_nif_length,
  zwave_home_id_t nwi_home_id);

/**
 * @brief Structure holding the application callback functions.
 *
 * The application callback functions must be registered to the Z-Wave API with
 * zwapi_init(). The functions will be invoked by the Z-Wave API at specific
 * times.
 *
 * @zgw_name SerialAPI_Callbacks
 */
typedef struct zwapi_callbacks {
  application_command_handler_function application_command_handler;
  application_controller_update_function application_controller_update;
  application_command_handler_function application_command_handler_bridge;
  void (*zwapi_started)(const uint8_t *pData, uint8_t pLen);
  void (*poll_request)();
} zwapi_callbacks_t;

/**
 * Initialize the Z-Wave API module with all the callbacks needed for operation.
 *
 * @param serial_port is the name of the serial port to use (on Linux i.e.,
 * "/dev/ttyS1")
 * @param serial_fd pointer where the serial file descriptor will be written
 * @param callbacks is a callback function to be used by the \ref ZWAPI.
 * @returns \ref SL_STATUS_OK if all initialization was successfully completed. It
 * includes initializing the Z-Wave module serial port, reading the module's
 * capabilities and its version. \ref SL_STATUS_FAIL otherwise
 *
 * @zgw_name SerialAPI_Init
 */
sl_status_t zwapi_init(const char *serial_port,
                       int *serial_fd,
                       const zwapi_callbacks_t *callbacks);

/**
 * @brief Shut down the Z-Wave API and close the communication ports.
 *
 * @zgw_name SerialAPI_Destroy
 */
void zwapi_destroy(void);

/**
 * @brief Enable logging of serial data to a file.
 *
 * The log will append to the file, it is up to the user to handle log rotation,
 * free disk space monitoring etc.
 *
 * @param filename file to log serial trace to, log will append to this file.
 * @return \ref SL_STATUS_OK on success.
 * @return \ref SL_STATUS_ALREADY_INITIALIZED if log to file is already enabled.
 * @return \ref SL_STATUS_FAIL if other failure (e.g. folder doesn't exist,
 *              lacking permissions, etc.).
 */
sl_status_t zwapi_log_to_file_enable(const char *filename);

/**
 * @brief Disable/stop logging serial data to file.
 *
 * This will stop the logging of serial data to file.
 *
 * @return \ref SL_STATUS_OK on success
 */
sl_status_t zwapi_log_to_file_disable();

/**
 * @brief Main poll loop for the Z-Wave API
 *
 * @returns returns true if there are still more frames to process in the
 * receive queue.
 *
 * This function must be called from some application main loop. This is what
 * drives the serial data receive handling. zwapi_poll() checks the serial port
 * for new data and handles timers and all other async callbacks. I.e., all
 * Z-Wave API callbacks are called from within this function.
 *
 * TODO: It would be nice to have zwapi_poll return the minimum number of system
 * ticks to parse before a timer times out. In this way we don't need to call
 * zwapi_poll unless we got a receive interrupt from the uart or if a timer has
 * timed out. This means that the system would be able to go into low power mode
 * between zwapi_poll calls.
 *
 * @zgw_name SerialAPI_Poll
 */
bool zwapi_poll(void);

/**
 * @brief Check if a command is supported by the connected Z-Wave module.
 *
 * @param func_id The FUNC_ID of the command to check, see zwapi_func_ids.h
 * @returns true if the command is supported
 * @returns false if the command is NOT supported
 *
 * @note A Z-Wave API session must have been opened with zwapi_init() before
 * calling this function.
 *
 * @zgw_name SerialAPI_SupportsCommand_func
 */
bool zwapi_support_command_func(uint8_t func_id);

/**
 * @brief Check if one of the serial API setup commands is supported.
 *
 * @param setup_cmd The command from @ref serial_api_setup_cmd_t to check.
 * @returns true if the command is supported, false otherwise.
 *
 * @zgw_name SupportsSerialAPISetup_func
 */
bool zwapi_support_setup_func(serial_api_setup_cmd_t setup_cmd);

/**
 * @brief Refresh the whole zwapi_chip_data_t structure from the
 * Z-Wave module.
 *
 * This function is called at initialization, but should also be called
 * if the Z-Wave module may change capabilities, for example after firmware
 * update or network inclusion.
 *
 * @returns SL_STATUS_OK if the refresh operation was successful.
 * @returns SL_STATUS_FAIL otherwise
 */
sl_status_t zwapi_refresh_capabilities(void);

/**
 * @brief Get zwave_api initialization data from the remote side (Enhanced Z-Wave
 * module).
 *
 * @param  ver  is the zwave_api version
 * @param capabilities  is the capabilities flag (GET_INIT_DATA_FLAG_xxx):
 *  - If @ref GET_INIT_DATA_FLAG_END_NODE_API is set, an End Node API is available,
 *    otherwise it is a Controller Node API
 *  - If @ref GET_INIT_DATA_FLAG_TIMER_SUPPORT is set then timer functions are
 *    supported.
 *  - If @ref GET_INIT_DATA_FLAG_SECONDARY_CTRL is set it's a secondary
 *    controller, otherwise it's a primary controller.
 *  - If @ref GET_INIT_DATA_FLAG_IS_SUC is set this node is SUC, otherwise not
 *    SUC.
 * @param len is the number of bytes in nodesList
 * @param node_list is the Bitmask list with nodes known by Z-Wave module
 * @param chip_type is the Z-Wave radio module  type
 * @param chip_version is the Z-Wave radio module  version
 * @returns SL_STATUS_OK if the initialization data was retrieved and parsed
 * correctly. SL_STATUS_FAIL otherwise
 *
 * @zgw_name SerialAPI_GetInitData
 */
sl_status_t zwapi_get_init_data(uint8_t *ver,
                                uint8_t *capabilities,
                                uint8_t *len,
                                zwave_nodemask_t node_list,
                                uint8_t *chip_type,
                                uint8_t *chip_version);

/**
 * @brief Refresh the node list from the Z-Wave protocol and copy in
 * into the node_list pointer.
 *
 * @param node_list is a zwapi_nodemask_t, where the data will be copied.
 * If this parameter is set to NULL, the node list will still be refreshed
 * from the Z-Wave module and can be retrieved subsequently with
 * @ref zwapi_get_chip_data
 * @returns SL_STATUS_OK if the node list was retrieved from the Z-Wave module.
 * SL_STATUS_FAIL otherwise
 */
sl_status_t zwapi_get_node_list(zwave_nodemask_t node_list);

/**
 * @brief Look up the cached chip type and chip version.
 *
 * @note Only valid after zwapi_get_init_data() has been called.
 *
 * @param type returns argument for the chip type.
 * @param version returns argument for the chip version.
 *
 * @zgw_name SerialAPI_GetChipTypeAndVersion
 */
void zwapi_get_chip_type_version(uint8_t *type, uint8_t *version);

/**
 * @brief Get the application version of the serial API on the Z-Wave module.
 *
 * @param major pointer to variable that should contain the major version.
 * @param minor pointer to variable that should contain the minor version.
 *
 * @zgw_name Get_SerialAPI_AppVersion
 */
void zwapi_get_app_version(uint8_t *major, uint8_t *minor);

/**
 * @brief Return all information known about the chip after initialization.
 *
 * @note Only valid after \ref zwapi_init() has returned SL_STATUS_OK
 *
 * @param user_chip_data address of the zwapi_chip_data_t to copy the data
 * returned by the module.
 */
void zwapi_get_chip_data(zwapi_chip_data_t *user_chip_data);

/**
 * @brief returns the cached value of the library type for the module
 *
 * @note Only valid after \ref zwapi_init() has returned SL_STATUS_OK
 *
 * @param zwapi_library_type_t value of the library type for the chip
 */
zwapi_library_type_t zwapi_get_library_type();

/**
 * @brief Retrieves versioning data from the Z-Wave API Module
 *
 * @returns SL_STATUS_OK or SL_STATUS_FAIL.
 *
 * @zgw_name ZW_GetProtocolVersion
 */
sl_status_t zwapi_get_protocol_version(
  zwapi_protocol_version_information_t *protocol_version);

/**
 * @brief Reads the currently configured Z-Wave Long Range Channel
 *
 * @returns zwave_lr_channel_t value for the configured Z-Wave Long Range channel.
 */
zwave_lr_channel_t zwapi_get_zwave_lr_channel();

/**
 * @brief Sets the desired Z-Wave Long Range Channel configuration
 *
 * @param new_channel value of the desired Z-Wave Long Range channel.
 * @returns sl_status_t indicating the outcome of the command.
 * SL_STATUS_OK if the command was delivered to the Z-Wave API
 * (but it does guarantee that the channel setting was accepted)
 * SL_STATUS_NOT_SUPPORTED if the Z-Wave API command is not supported.
 *
 * @note @ref zwapi_get_zwave_lr_channel should be called subsequently to verify
 * if the channel configuration was accepted
 */
sl_status_t zwapi_set_zwave_lr_channel(zwave_lr_channel_t new_channel);

// CLEANUP: Moved form sdk_versioning.h
/**
 * @brief Convert protocol version string to Z-Wave SDK version string
 *
 * @param version_string string created by (first argument of)
 * zwapi_get_zw_library_version(version_string)
 *
 * @returns returns SDK version (e.g., "6.81.00")
 *
 * @zgw_name GenerateSdkVersionFromProtocolVersion
 */
const char *
  zwapi_generate_sdk_version_from_protocol_version(const char *version_string);

/**
 * @brief Convert Z-Wave SDK version to the NVM ID.
 *
 * @param sdk_ver returned by zwapi_generate_sdk_version_from_protocol_version()
 * @param library_type returned by zwapi_get_zw_library_version()
 *
 * @returns nvm_id (e.g., bridge6.81)
 *
 * @zgw_name GenerateNvmIdFromSdkVersion
 */
const char *
  zwapi_generate_nvm_id_from_sdk_version(const char *sdk_ver,
                                         zwapi_library_type_t library_type);

/// @} End of defgroup ZWAPI_INIT

#ifdef __cplusplus
}
#endif

#endif  // ZWAPI_INIT_H
