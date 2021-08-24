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
 * @file zwapi_protocol_basis.h
 * @brief Z-Wave API for basis controller/end node application interface.
 */

#ifndef ZWAPI_PROTOCOL_BASIS_H
#define ZWAPI_PROTOCOL_BASIS_H

#include <stdint.h>
#include <stdbool.h>
#include "sl_status.h"
#include "zwave_utils.h"
#include "zwave_rf_region.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup ZWAPI_BASIS Basis API
 * @ingroup ZWAPI
 * @brief Functions in the Z-Wave basis API
 *
 * Interface functions in this module are implemented within all Z-Wave nodes.
 * @{
 */

/// Bitflag definitions for FUNC_ID_SERIAL_API_SETUP command
typedef enum {
  SERIAL_API_SETUP_CMD_UNSUPPORTED                   = 0x00,
  SERIAL_API_SETUP_CMD_SUPPORTED                     = 0x01,
  SERIAL_API_SETUP_CMD_TX_STATUS_REPORT              = 0x02,
  SERIAL_API_SETUP_CMD_TX_POWERLEVEL_SET             = 0x04,
  SERIAL_API_SETUP_CMD_TX_POWERLEVEL_GET             = 0x08,
  SERIAL_API_SETUP_CMD_MAXIMUM_PAYLOAD_SIZE_GET      = 0x10,
  SERIAL_API_SETUP_CMD_LR_MAXIMUM_PAYLOAD_SIZE_GET   = 0x11,
  SERIAL_API_SETUP_CMD_RF_REGION_GET                 = 0x20,
  SERIAL_API_SETUP_CMD_RF_REGION_SET                 = 0x40,
  SERIAL_API_SETUP_CMD_NODEID_BASETYPE_SET           = 0x80
} serial_api_setup_cmd_t;

/// @name zwapi_nodeid_basetype_t
/// These definitions represent possible node id basetype settings for the Z-Wave module
/// They are used with zwapi_set_node_id_basetype()
///@{
typedef enum {
  NODEID_8BITS = 1,
  NODEID_16BITS = 2
} zwave_node_id_basetype_t;
///@}

/// @name zwapi_get_zw_protocol_status() return value masks
///@{
#define ZW_PROTOCOL_STATUS_ROUTING 0x01
#define ZW_PROTOCOL_STATUS_SUC     0x02
///@}

/// Bit mask used as parameters for the API ZW_SetSleepMode to indicate which
/// interrupt to mask out
#define ZW_INT_MASK_EXT1 0x02

#define ZW_AV_VERSION_MAJOR 1
#define ZW_AV_VERSION_MINOR 0

/// @name Z-Wave Library Chip type
///@{
#define ZW_500_SERIES_CHIP_TYPE     5
#define ZW_500_SERIES_CHIP_REVISION 0
///@}

/// @name Chip type definitions for 700-series
///@{
#define ZW_GECKO_CHIP_TYPE     7
#define ZW_GECKO_CHIP_REVISION 0
///@}

/// Power level structure used by zwapi_set_tx_power_level() and
/// zwapi_set_tx_power_level().
typedef struct {
  int8_t normal;  ///< TX power level at normal power
  int8_t
    measured0dBm;  ///< Measured output power from antenna when normal is set to 0
} tx_power_level_t;

/// RF power level values used with zwapi_set_rf_power_level() and
/// zwapi_get_rf_power_level()
typedef enum {
  NORMAL_POWER = 0,            ///< Max power possible
  MINUS_1_DBM,                 ///< Normal power - 1dB (mapped to minus2dB)
  MINUS_2_DBM,                 ///< Normal power - 2dB
  MINUS_3_DBM,                 ///< Normal power - 3dB (mapped to minus4dB)
  MINUS_4_DBM,                 ///< Normal power - 4dB
  MINUS_5_DBM,                 ///< Normal power - 5dB (mapped to minus6dB)
  MINUS_6_DBM,                 ///< Normal power - 6dB
  MINUS_7_DBM,                 ///< Normal power - 7dB (mapped to minus8dB)
  MINUS_8_DBM,                 ///< Normal power - 8dB
  MINUS_9_DBM,                 ///< Normal power - 9dB (mapped to minus10dB)
  MINIMUM_POWER = MINUS_9_DBM  ///< Equal to minus9dBm
} rf_power_level_t;

/// @name Device option mask values
///@{
#define APPLICATION_NODEINFO_NOT_LISTENING          0x00
#define APPLICATION_NODEINFO_LISTENING              0x01
#define APPLICATION_NODEINFO_OPTIONAL_FUNCTIONALITY 0x02
#define APPLICATION_FREQ_LISTENING_MODE_1000ms      0x10
#define APPLICATION_FREQ_LISTENING_MODE_250ms       0x20
///@}

/**
 * @brief Initialize the Z-Wave RF chip.
 *
 * @param mode is the Z-Wave Radio receiver mode
 * = true : Set the RF chip in receive mode and starts the data sampling. \n
 * = false : Set the RF chip in power down mode. \n
 *
 * @returns returns SL_STATUS_OK if operation was executed successfully,
 * SL_STATUS_FAIL otherwise.
 *
 * @zgw_name ZW_SetRFReceiveMode
 */
sl_status_t zwapi_set_rf_receive_mode(uint8_t mode);

/**
 * @brief Create and transmit a "Node Information" frame.
 *
 * @param destNode is the destination Node ID (ZWAVE_BROADCAST_NODE_ID == all nodes)
 * @param txOptions is transmit option flags. (see zwapi_send_data())
 * @param completedFunc is the transmit completed callback function. For
 * information about the txStatus parameter see zwapi_send_data().
 * @returns true if frame was put in the transmit queue.
 * @returns false if frame was not put in transmit queue (possibly due to
 * transmit queue overflow). The completedFunc callback will *not* be called.
 *
 * The Z Wave transport layer builds a frame, requests application node
 * information (see ApplicationNodeInformation), and queues the "Node Information"
 * frame for transmission. The completed call back function (completedFunc) is
 * called when the transmission is complete.
 *
 * The Node Information Frame is a protocol frame and will therefore not be
 * directly available to the application on the receiver. The API call
 * zwapi_set_learn_mode() can be used to instruct the protocol to pass the Node
 * Information Frame to the application.
 *
 * When zwapi_send_node_information() is used in learn mode for adding or
 * removing the node from the network the transmit option
 * TRANSMIT_OPTION_LOW_POWER should NOT be used.
 *
 * @note zwapi_send_node_information uses the transmit queue in the API, so
 * using other transmit functions before the complete callback has been called
 * by the API is not recommended.
 *
 * Timeout: 65 s
 * Exception recovery: Resume normal operation, no recovery needed
 *
 * @serial_tx{REQ | 0x12 | destNode | txOptions | funcID}
 *
 * @serial_rx{RES | 0x12 | retVal}
 *
 * @serial_rx{REQ | 0x12 | funcID | txStatus}
 *
 * @zgw_name ZW_SendNodeInformation
 */
sl_status_t zwapi_send_node_information(zwave_node_id_t destNode,
                                        uint8_t txOptions,
                                        void (*completedFunc)(uint8_t));

/**
 * @brief Get the Z-Wave library basis version.
 *
 * @param dst pointer to buffer where version string will be  copied to. The
 * buffer must be at least 14 bytes long. If it is NULL, nothing will be copied.
 *
 * @returns the Library type ZW_LIB_xxx
 *
 * @zgw_name ZW_Version
 */
uint8_t zwapi_get_zw_library_version(uint8_t *dst);

/**
 * @brief Get the Z-Wave library type.
 *
 * @note: Use this function only if necessary to query a fresh value.
 * zwapi_get_library_type will provide the cached value at initialization
 *
 * @returns Z-Wave library type:
 *   - @ref zwapi_library_type_t
 *
 * @zgw_name ZW_Type_Library
 */
uint8_t zwapi_get_zw_library_type(void);

/**
 * @brief Set the power level to used in RF transmissions.
 *
 * @param powerLevel the power level to use in RF transmission.
 * @returns rf_power_level_t the power level set in the Z-Wave Radio Module.
 *
 * The RF power is dependent on the settings for transmit power level in
 * App_RFSetup.a51. If this value is changed from using the default library
 * value, the resulting power levels might differ from the intended values. The
 * returned value is however always the one used.
 *
 * @note This function should only be used in an install/test link situation and
 * the power level should always be set back to NORMAL_POWER when the testing is
 * done.
 *
 * @serial_tx{REQ | 0x17 | powerLevel}
 *
 * @serial_rx{RES | 0x17 | retVal}
 *
 * @zgw_name ZW_RFPowerLevelSet
 */
rf_power_level_t zwapi_set_rf_power_level(rf_power_level_t powerLevel);

/**
 * @brief Get the current power level used in RF transmitting.
 *
 * \returns rf_power_level_t the power level currently in effect during RF
 * transmissions.
 *
 * \note This function should only be used in an install/test link situation.
 *
 * @serial_tx{REQ | 0xBA}
 *
 * @serial_rx{RES | 0xBA | powerlevel}
 *
 * @zgw_name ZW_RFPowerLevelGet
 */
rf_power_level_t zwapi_get_rf_power_level(void);

/**
 * @brief Configure whether zwapi_send_data callbacks should contain tx_status
 * report information.
 *
 * @param enable true will enabled tx_status reporting, false will disable it.
 *
 * @returns SL_STATUS_OK if the tx_status_report was set successfully.
 * @returns SL_STATUS_NOT_SUPPORTED if the Z-Wave Module does not support this function
 * @returns SL_STATUS_FAIL if the setting could not be changed.
 */
sl_status_t zwapi_set_tx_status_reporting(bool enable);

/**
 * @brief Set the current TX power level.
 *
 * @param zw_txpowerlevel TX power level settings struct with the following
 * members:
 *  - @b normal The power level used when transmitting frames at normal power.
 *    The power level is in deci dBm. E.g. 1dBm output power will be 10 in
 *    NormalTxPower and -2dBm will be -20 in NormalTxPower.
 *  - @b measured0dBm The output power measured from the antenna when
 *    NormalTxPower is set to 0dBm. The power level is in deci dBm. E.g. 1dBm
 *    output power will be 10 in Measured0dBmPower and -2dBm will be -20 in
 *    Measured0dBmPower.
 * @returns SL_STATUS_OK if the TX power level was set successfully.
 * @returns SL_STATUS_NOT_SUPPORTED if the Z-Wave module does not
 * support this function.
 * @returns SL_STATUS_FAIL if the TX power level could not be set.
 *
 * @note This API is available as of serial API version 7
 *
 * @serial_tx{REQ | 0x0B | 0x04 | NormalTxPower | Measured0dBmPower}
 *
 * @serial_rx{RES | 0x0B | 0x04 | CmdRes}
 *
 * @zgw_name ZW_TXPowerLevelSet
 */
sl_status_t zwapi_set_tx_power_level(tx_power_level_t zw_txpowerlevel);

/**
 * @brief Get the current TX power level in effect.
 *
 * @returns TX power level settings struct with the following members:
 *  - @b normal The power level used when transmitting frames at normal power.
 *    The power level is in deci dBm. E.g. 1dBm output power will be 10 in
 *    NormalTxPower and -2dBm will be -20 in NormalTxPower.
 *  - @b measured0dBm The output power measured from the antenna when
 *    NormalTxPower is set to 0dBm. The power level is in deci dBm. E.g. 1dBm
 *    output power will be 10 in Measured0dBmPower and -2dBm will be -20 in
 *    Measured0dBmPower.
 *
 * @note This API is available as of serial API version 7
 *
 * @serial_tx{REQ | 0x0B | 0x08}
 *
 * @serial_rx{RES | 0x0B | 0x08 | NormalTxPower | Measured0dBmPower}
 *
 * @zgw_name ZW_TXPowerLevelGet
 */
tx_power_level_t zwapi_get_tx_power_level(void);

/**
 * @brief Set the current RF region.
 *
 * @param rfregion the RF region
 * @returns SL_STATUS_OK the RF region was set successfully.
 * @returns SL_STATUS_FAIL the RF region was *not* set.
 *
 * @serial_tx{REQ | 0x0B | 0x40 | RFRegion}
 *
 * @serial_rx{RES | 0x0B | 0x40 | CmdRes}
 *
 * @zgw_name ZW_RFRegionSet
 */
sl_status_t zwapi_set_rf_region(zwave_rf_region_t rfregion);

/**
 * @brief Get the current RF region.
 *
 * @returns zwapi_rf_region_t the RF region currently in effect. REGION_UNDEFINED if
 * unable to get RF region.
 *
 * @serial_tx{REQ | 0x0B | 0x20}
 *
 * @serial_rx{RES | 0x0B | 0x20 | RFRegion}
 *
 * @zgw_name ZW_RFRegionGet
 */
zwave_rf_region_t zwapi_get_rf_region(void);

/**
 * @brief Get the maximum payload size allowed for sending frames in Z-Wave
 *
 * @param maximum_size The value to indicate maximum payload size
 * @returns SL_STATUS_OK in case of success, SL_STATUS_FAIL otherwise.
 *
 */
sl_status_t zwapi_get_maximum_payload_size(uint8_t *maximum_size);

/**
 * @brief Get the maximum payload size allowed for sending frames in Z-Wave Long Range
 *
 * @param maximum_size The value to indicate maximum payload size
 * @returns SL_STATUS_OK in case of success, SL_STATUS_FAIL otherwise.
 *
 */
sl_status_t zwapi_get_lr_maximum_payload_size(uint8_t *maximum_size);

/**
 * @brief Set Z-Wave node ID basetype, either 8 bits or 16 bits used to store a node ID
 *
 * @param basetype zwave_node_id_basetype_t the basetype to be set, NODEID_8BITS or NODEID_16BITS
 * @returns SL_STATUS_OK in case of success, SL_STATUS_FAIL otherwise.
 *
 * @zgw_name SerialAPI_GetLRNodeList
 */
sl_status_t zwapi_set_node_id_basetype(zwave_node_id_basetype_t basetype);

/**
 * @brief Get the current node ID basetype setting.
 *
 * @returns basetype zwave_nodeid_basetype_t the current node ID basetype, NODEID_8BITS or NODEID_16BITS
 *
 */
zwave_node_id_basetype_t zwapi_get_node_id_basetype();

/**
 * @brief Reset the Z-Wave module.
 *
 * @note USB modules will do a disconnect - connect when this command is issued.
 * This means that the module may get a new address on the USB bus. This will
 * make the old file handle to the USB serial interface invalid.
 *
 * @note Recent Z-Wave versions (at least 6.80 and 7.00) will return
 * FUNC_ID_SERIAL_API_STARTED once restarted.
 *
 * @serial_tx{REQ | 0x08}
 *
 * @zgw_name ZW_SoftReset
 */
sl_status_t zwapi_soft_reset(void);

/**
 * @brief Send a test frame to specified NodeID.
 *
 * @param node_id           the NodeID destination the test frame
 *                          will be transmitted to.
 * @param power_level       the power level to use for the RF transmission.
 * @param callback_function is the callback function that will be called
 *                          when the operation of this function is done.
 *                          For information about the tx_status parameter,
 *                          refer to \ref zwapi_send_data
 *
 * @returns SL_STATUS_OK if the frame was sent.
 * @returns SL_STATUS_FAIL if the frame was *not* sent.
 *
 * Send a test frame directly to nodeID without any routing, RF transmission
 * power is previously set to power level by calling zwapi_set_rf_power_level().
 * The test frame is acknowledged at the RF transmission power level indicated by
 * the parameter power level by nodeID (if the test frame got through).This test
 * will be done using 9600 kbit/s transmission rate.
 *
 * @note This function should only be used in an install/test link situation.
 *
 * Timeout: 200 ms
 * Exception recovery: Resume normal operation, no recovery needed
 *
 * @zgw_name ZW_SendTestFrame
 */
sl_status_t zwapi_send_test_frame(zwave_node_id_t node_id,
                                  rf_power_level_t power_level,
                                  void (*callback_function)(uint8_t tx_status));

/**
 * @brief Report the status of the Z-Wave protocol.
 *
 * @returns 0 if protocol is idle.
 * @returns @ref ZW_PROTOCOL_STATUS_ROUTING if protocol is analyzing the routing table.
 * @returns @ref ZW_PROTOCOL_STATUS_SUC if SUC sends pending updates.
 *
 * The function returns a mask telling which protocol function is currently running
 *
 * @serial_tx{REQ | 0xBF}
 *
 * @serial_rx{RES | 0xBF | retVal}
 *
 * @zgw_name ZW_GetProtocolStatus
 */
uint8_t zwapi_get_zw_protocol_status(void);

/**
 * @brief Enable the Watchdog.
 *
 * @returns SL_STATUS_OK or SL_STATUS_FAIL
 *
 * @zgw_name ZW_WatchDogEnable
 */
sl_status_t zwapi_enable_watchdog();

/**
 * @brief Disable the Watchdog.
 *
 * @returns SL_STATUS_OK or SL_STATUS_FAIL
 *
 * @zgw_name ZW_WatchDogDisable
 */
sl_status_t zwapi_disable_watchdog();

/**
 * @brief Enable Watchdog and start kicking it in the Z-Wave chip.
 *
 * @returns SL_STATUS_OK or SL_STATUS_FAIL
 *
 * @serial_tx{REQ | 0xD2}
 *
 * @zgw_name SerialAPI_WatchdogStart
 */
sl_status_t zwapi_start_watchdog();

/**
 * @brief Generate an array of random bytes using the random number generator
 * (RNG) on the Z-Wave module.
 *
 * @param random_buffer pointer to buffer, which should receive the random bytes.
 * @param number_of_random_bytes Number of random bytes to get into
 * random_buffer. Max number is 32.
 * @returns SL_STATUS_OK in case of success, SL_STATUS_FAIL otherwise.
 *
 * @serial_tx{REQ | 0x1C | noRandomBytes}
 *
 * @serial_rx{RES | 0x1C | randomGenerationSuccess | noRandomBytesGenerated | randombytes[]}
 *
 * @zgw_name ZW_GetRandomWord (ZW_GetRandomArray)
 */
sl_status_t zwapi_get_random_word(uint8_t *random_buffer,
                                  uint8_t number_of_random_bytes);

/**
 * @brief Generate a random byte using the random number generator (RNG) on the
 * Z-Wave module
 *
 * @param user_byte pointer to uint8_t variable where the random number will be
 * copied. Nothing is copied if SL_STATUS_FAIL is returned.
 * @returns SL_STATUS_OK in case of success, SL_STATUS_FAIL otherwise.
 *
 * @serial_tx{HOST->ZW: REQ | 0x1D}
 * @serial_tx{ZW->HOST: RES | 0x1D | rndNo}
 *
 * @zgw_name ZW_Random(ZW_GetRandomArray)
 */
sl_status_t zwapi_get_random_byte(uint8_t *user_byte);

/**
 * @brief Transmit a request for inclusion via an explore frame.
 *
 * @returns SL_STATUS_OK if the Inclusion request queued for transmission
 * @returns SL_STATUS_FAIL if the node is not in learn mode
 *
 * This function sends out an explorer frame requesting inclusion into a
 * network. If the inclusion request is accepted by a controller in network wide
 * inclusion mode, the application on this node will get notified through
 * the callback from the zwapi_set_learn_mode() function. Once a callback is
 * received from zwapi_set_learn_mode() saying that the inclusion process has
 * started, the application should not make further calls to this function.
 *
 * @note Recommend not to call this function more than once every 4 seconds.
 *
 * @serial_tx{REQ | 0x5E}
 *
 * @serial_rx{RES | 0x5E | retVal}
 *
 * @zgw_name ZW_ExploreRequestInclusion
 */
sl_status_t zwapi_explore_request_inclusion();

/**
 * @brief Transmit a request for exclusion via an explore frame.
 *
 * This function sends out an explorer frame requesting exclusion from the
 * network. If the exclusion request is accepted by a controller in network wide
 * exclusion mode, the application on this node will get notified through
 * the callback from the zwapi_set_learn_mode() function. Once a callback is
 * received from zwapi_set_learn_mode() saying that the exclusion process has
 * started, the application should not make further calls to this function.
 *
 * @note Recommend not to call this function more than once every 4 seconds.
 *
 * @returns SL_STATUS_OK if the Exclusion request queued for transmission
 * @returns SL_STATUS_FAIL if the node  is not in learn mode
 *
 * @serial_tx{REQ | 0x5E}
 *
 * @serial_rx{RES | 0x5E | retVal}
 *
 * @zgw_name ZW_ExploreRequestExclusion
 */
sl_status_t zwapi_explore_request_exclusion();

/**
 * @brief Get the latest value of the background RSSI.
 *
 * @param rssi_values pointer to array where RSSI values are stored.
 * @param values_length length in bytes of the rssi_values array
 *
 * @serial_tx{(no arguments)}
 *
 * @serial_rx{RES | RSSI Ch0 | RSSI Ch1 | RSSI Ch2 (3CH systems only)}
 *
 * @zgw_name ZW_GetBackgroundRSSI
 */
sl_status_t zwapi_get_background_rssi(uint8_t *rssi_values,
                                      uint8_t *values_length);

/**
 * @brief Set ApplicationNodeInformation data to be used in subsequent calls to
 * zwapi_send_node_information().
 *
 * @param listening Set to APPLICATION_NODEINFO_LISTENING if this is a listening
 * node, otherwise set to APPLICATION_NODEINFO_NOT_LISTENING.
 * @param node_type the Z-Wave Device type (the .basic member is not used here)
 * @param nodeParm the Device parameter buffer
 * @param parmLength the number of Device parameter bytes
 *
 * @zgw_name SerialAPI_ApplicationNodeInformation
 */
sl_status_t zwapi_set_application_node_information(uint8_t listening,
                                                   node_type_t node_type,
                                                   const uint8_t *nodeParm,
                                                   uint8_t parmLength);

/**
 * @brief Get Z-Wave Long Range node list in ZW_LR_NODEMASK_OFFSET offset of
 * zwave_nodemask_t
 *
 * This function copies Long Range node list bimask at ZW_LR_NODEMASK_OFFSET
 * offest of zwave_nodemask_t
 *
 * @param number_of_bytes Number of bytes in the returned node list
 * @param lr_node_list array of type zwave_nodemask_t
 * @returns SL_STATUS_OK in case of success, SL_STATUS_FAIL otherwise.
 *
 * @zgw_name SerialAPI_GetLRNodeList
 */
sl_status_t zwapi_get_long_range_nodes(uint16_t *number_of_bytes,
                                       zwave_nodemask_t lr_node_list);

/**
 * @brief Get the entire list of nodes in our network (both Z-Wave and Z-Wave Long Range)
 *
 * @param node_list array of type zwave_nodemask_t
 * @returns SL_STATUS_OK in case of success, SL_STATUS_FAIL otherwise.
 *
 */
sl_status_t zwapi_get_full_node_list(zwave_nodemask_t node_list);
/**
 * @brief Get the Z-Wave Long Range radio channel in use
 *
 * @param lr_channel The Z-Wave Long Range radio channel currecntly in use
 * @returns SL_STATUS_OK in case of success, SL_STATUS_FAIL otherwise.
 *
 * @zgw_name GetLongRangeChannel
 */
sl_status_t zwapi_get_long_range_channel(uint8_t *lr_channel);

/**
 * @brief Set the Z-Wave Long Range radio channel in use
 *
 * @param lr_channel The value intended to set for current Z-Wave Long Range radio channel
 * @returns SL_STATUS_OK in case of success, SL_STATUS_FAIL otherwise.
 *
 * @zgw_name SetLongRangeChannel
 */
sl_status_t zwapi_set_long_range_channel(uint8_t lr_channel);

/**
 * @brief Enable Auto Programming mode
 *
 * @returns SL_STATUS_OK or SL_STATUS_FAIL.
 *
 * @zgw_name ZW_AutoProgrammingEnable
 */
sl_status_t zwapi_enable_auto_program_mode(void);

/**
 * @brief Return whether the Z-Wave module supports Long Range
 *
 * @returns True or False
 *
 * @zgw_name SerialAPI_SupportsLR
 */
bool zwapi_supports_long_range();

/// @} End of defgroup ZWAPI_BASIS

#ifdef __cplusplus
}
#endif

#endif  // ZWAPI_PROTOCOL_BASIS_H
