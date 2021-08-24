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

#include <stdint.h>
/**
 * @defgroup zpc_crc16 ZPC CRC 16
 * @ingroup zpc_utils
 * @brief CRC16 calculation function
 *
 * @{
 */
#ifndef ZPC_CRC16_H
#define ZPC_CRC16_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculation of CRC16-CCITT with Polynomial 0x1021 on data
 *
 * @param crc16             Initial value for CRC 16 calculation algorithm
 * @param data              data
 * @param data_length       Length of the data.
 * @return calculated crc16
 */

uint16_t
  zpc_crc16(uint16_t crc16, const uint8_t *data_buf, unsigned long data_len);
#ifdef __cplusplus
}
#endif

#endif  // ZPC_CRC16_H
/** @} end zpc_utils */
