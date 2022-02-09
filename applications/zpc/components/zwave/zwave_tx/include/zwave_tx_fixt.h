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
 * @defgroup zwave_tx_fixt Z-Wave TX Fixture
 * @ingroup zwave_tx
 * @brief Z-Wave TX fixture starts the Z-Wave TX process
 *
 * The Z-Wave TX fixture is responsible for starting up the Z-Wave TX process
 *
 * @{
 */
#include <stdbool.h>
#include "sl_status.h"

#ifndef ZWAVE_TX_FITX_H
#define ZWAVE_TX_FITX_H

#ifdef __cplusplus
extern "C" {
#endif

/** Setup fixture for Z-Wave TX.
 *
 * This setup function start the Z-Wave TX Process
 *
 * @returns SL_STATUS_OK if successful and SL_STATUS_FAIL if an error occurred
 */
sl_status_t zwave_tx_fixt_setup(void);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_TX_FITX_H
/** @} end zwave_tx_fitx */
