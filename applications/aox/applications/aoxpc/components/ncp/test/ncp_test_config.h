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
 * @brief Bluetooth Network Co-Processor test config.
 */

#ifndef NCP_TEST_CONFIG_H
#define NCP_TEST_CONFIG_H

#include "sl_log.h"
#include "process.h"

// Helper macros
#define ncp_test_log(...)        sl_log_info("NCP test", __VA_ARGS__)
#define ncp_test_terminate(sc)   process_post(PROCESS_BROADCAST, PROCESS_EVENT_EXIT, 0)

#endif // NCP_TEST_CONFIG_H
