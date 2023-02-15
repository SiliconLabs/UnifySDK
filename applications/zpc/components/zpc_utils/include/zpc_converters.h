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
 * @defgroup zpc_utils ZPC Utilities
 * @ingroup zpc_components
 * @brief Various conversion functions and macros for ZPC
 *
  * @{
 */

#ifndef ZPC_CONVERTERS_H
#define ZPC_CONVERTERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_status.h"
#include "zwave_controller_types.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define DSK_STR_LEN \
  (sizeof("xxxxx-xxxxx-xxxxx-xxxxx-xxxxx-xxxxx-xxxxx-xxxxx") + 1)

/**
 * @brief Convert DSK string to uint8_t array representation.
 *
 * @param src DSK string formatted as
 *            "xxxxx-xxxxx-xxxxx-xxxxx-xxxxx-xxxxx-xxxxx-xxxxx"
 *            it doesn't need to 5 characters per entry, i.e.,
 *            "1-12345-33-23423-0-1-3-4" is supported
 *            MUST BE ZERO TERMINATED
 * @param dst destination buffer to store zwave_dsk_t in
 * @return sl_status_t SL_STATUS_OK if success
 */
sl_status_t zpc_converters_dsk_str_to_internal(const char *src,
                                               zwave_dsk_t dst);

/**
 * @brief Convert a zwave_dsk_t to a String representation.
 *
 * String representation in format
 * "xxxxx-xxxxx-xxxxx-xxxxx-xxxxx-xxxxx-xxxxx-xxxxx" as decimal numbers
 *
 * @param src DSK to write to string
 * @param dst Buffer to store string representation in
 * @param dst_max_len Size of the dst buffer to prevent overflow,
 *                    shall be >= DSK_STR_LEN
 * @return SL_STATUS_OK for success
 * @return SL_STATUS_WOULD_OVERFLOW if char buffer is too small
 */
sl_status_t zpc_converters_dsk_to_str(const zwave_dsk_t src,
                                      char *dst,
                                      size_t dst_max_len);

#ifdef __cplusplus
}
#endif
#endif  //ZWAVE_SMARTSTART_MANAGEMENT_H
/** @} end zpc_converters */
