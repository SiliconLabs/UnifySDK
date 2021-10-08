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
 * @file zcl_util.h
 * @defgroup zigpc_zcl_util ZigPC ZCL Utilities
 * @ingroup zigpc_components
 * @brief This component contains definitions and behaviours that is used to
 * manipulate ZCL data types.
 *
 * @{
 */

#ifndef ZIGPC_ZCL_UTIL_H
#define ZIGPC_ZCL_UTIL_H

#include "zigpc_common_zigbee.h"

#include "zcl_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ZCL frame control types
 *
 */
typedef enum {

  ZIGPC_ZCL_FRAME_TYPE_CMD_TO_SERVER
  = (ZCL_FRAME_CONTROL_FRAME_TYPE_CLUSTER
     | ZCL_FRAME_CONTROL_DIRECTION_CLIENT_TO_SERVER),
  ZIGPC_ZCL_FRAME_TYPE_GLOBAL_CMD_TO_SERVER
  = (ZCL_FRAME_CONTROL_FRAME_TYPE_GLOBAL_COMMAND
     | ZCL_FRAME_CONTROL_DIRECTION_CLIENT_TO_SERVER),

} zigpc_zcl_frame_type_t;

/**
 * @brief Structure used to pass data for ZCL frame build helpers.
 *
 * NOTE: The size of the data is inferred from the type set.
 * NOTE: The data passed in should not cross any contiki process contexts.
 */
typedef struct {
  zigpc_zcl_data_type_t type;
  const void *data;
} zigpc_zcl_frame_data_t;

/**
 * @brief Get data size of ZCL type passed in.
 *
 * @param type      ZCL data type to lookup
 * @return size_t   Number of bytes occupied by the data with ZCL type; if size
 * information is not found, 0 is returned.
 */
size_t zigpc_zcl_get_data_type_size(zigpc_zcl_data_type_t type);

/**
 * @brief Initialize the ZCL Frame for sending ZCL command.
 *
 * @param frame         Pointer to frame to be initialized.
 * @param command_id    ZCL Command ID to be populated in the frame.
 * @param frame_control ZCL frame control field to be populated.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NULL_POINTER if
 * if invalid pointers are passed in.
 */
sl_status_t zigpc_zcl_frame_init_command(zcl_frame_t *const frame,
                                         zcl_command_id_t command_id,
                                         uint8_t frame_control);

/**
 * @brief Populate the given ZCL frame with data. The data copied is dictated
 * by the type passed in. Once successfully copied into the frame, the buffer
 * size is incremented based on the size of the data type passed in.
 *
 * @param frame         Pointer to the ZCL frame to be populated.
 * @param type          Type of data to be transferred to the frame.
 * @param data          Pointer to the data to be transferred.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NULL_POINTER if
 * if invalid pointers are passed in, or SL_STATUS_WOULD_OVERFLOW if appending
 * data would cause buffer overflow.
 */
sl_status_t zigpc_zcl_frame_fill_data(zcl_frame_t *const frame,
                                      zigpc_zcl_data_type_t type,
                                      const void *data);

/**
 * @brief Populate the ZCL frame with an array of the given ZCL data type.
 * Once successfully copied into the frame, the buffer size is incremented
 * based on the size of the data type and array count passed in.
 *
 * @param frame         Pointer to the ZCL frame to be populated.
 * @param type          Type of data to be transferred to the frame.
 * @param type_count    Number of data elements to be compied.
 * @param data          Pointer to the data to be transferred.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NULL_POINTER if
 * if invalid pointers are passed in, SL_STATUS_WOULD_OVERFLOW if appending
 * data would cause buffer overflow, or SL_STATUS_INVALID_COUNT if the count
 * of elements is 0.
 */
sl_status_t zigpc_zcl_frame_fill_data_array(zcl_frame_t *const frame,
                                            zigpc_zcl_data_type_t type,
                                            size_t type_count,
                                            const void *data);

/**
 * @brief Build a ZCL command frame to be sent on the Zigbee network.
 *
 * NOTE: Ensure pointers to data passed through command argument list is valid
 * in the context that this function is called.
 *
 * @param frame               Pointer to ZCL frame container to be populated.
 * @param frame_type          ZCL frame information.
 * @param cluster_id          Cluster identifier.
 * @param command_id          Cluster command identifier.
 * @param command_arg_count   Command argument list count.
 * @param command_arg_list    Command argument type and data list.
 * arguments.
 * @return sl_status_t        SL_STATUS_OK on success, SL_STATUS_NULL_POINTER if
 * if invalid pointers are passed in, or other error related to the ZCL command
 * argument appending process.
 */
sl_status_t zigpc_zcl_build_command_frame(
  zcl_frame_t *const frame,
  const zigpc_zcl_frame_type_t frame_type,
  const zcl_cluster_id_t cluster_id,
  const zcl_command_id_t command_id,
  const size_t command_arg_count,
  const zigpc_zcl_frame_data_t *const command_arg_list);

/**
 * @brief zigpc_zcl_get_type_size - retrieve the size (in bytes) of
 * a given ZCL type
 *
 * @param type - the ZCL type
 *
 * @return the size in bytes of the requested type. returns 0 if
 * no type or an invalid type was found
 **/
size_t zigpc_zcl_get_type_size(zigpc_zcl_data_type_t type);

#ifdef __cplusplus
}
#endif

#endif /* ZIGPC_ZCL_UTIL_H */

/** @} end zigpc_zcl_util */
