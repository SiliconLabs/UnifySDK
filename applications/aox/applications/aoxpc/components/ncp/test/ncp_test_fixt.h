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
 * @brief Bluetooth Network Co-Processor test fixture.
 */

#ifndef NCP_TEST_FIXT_H
#define NCP_TEST_FIXT_H

#include "sl_status.h"

void ncp_test_config_init(void);
sl_status_t ncp_test_setup(void);

#endif // NCP_TEST_FIXT_H
