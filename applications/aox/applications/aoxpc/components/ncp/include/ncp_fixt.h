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
 * @brief Bluetooth Network Co-Processor fixture.
 */

#ifndef NCP_FIXT_H
#define NCP_FIXT_H

#include "sl_status.h"

/**
 * @brief Fixture for setting up the Bluetooth NCP component.
 *
 * @return SL_STATUS_OK on success
 */
sl_status_t ncp_fixt_setup(void);

/**
 * @brief Fixture for shutting down the Bluetooth NCP component.
 *
 * @return 0 on success
 */
int ncp_fixt_shutdown(void);

#endif // NCP_FIXT_H
