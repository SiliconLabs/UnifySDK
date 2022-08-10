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
 * @file
 * @brief Network Co-Processor core module.
 */

#ifndef AOXPC_NCP_CORE_H
#define AOXPC_NCP_CORE_H

#include <stdint.h>
#include "sl_status.h"

/**
 * @brief Getter for the NCP target device file descriptor.
 * 
 * @return File descriptor of the serial port or the TCP socket.
 */
int ncp_get_fd(void);

/**
 * @brief Transmit data to the NCP target.
 * 
 * @param len Data buffer length in bytes.
 * @param data Data buffer.
 */
void ncp_tx(uint32_t len, uint8_t *data);

/**
 * @brief Receive data from the NCP target.
 * 
 * @param len Data buffer length in bytes.
 * @param data Data buffer.
 * @return int32_t 
 */
int32_t ncp_rx(uint32_t len, uint8_t *data);

/**
 * @brief Check if readable data exists.
 * 
 * @return Number of bytes in the RX buffer, -1 on error.
 */
int32_t ncp_peek(void);

/**
 * @brief Flush the RX buffer.
 */
void ncp_flush(void);

#endif // AOXPC_NCP_CORE_H
