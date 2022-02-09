/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef TEST_ZIGPC_DISCOVERY_CALLBACKS_H
#define TEST_ZIGPC_DISCOVERY_CALLBACKS_H

#include "zigpc_discovery.h"

/**
 * @brief Test callback for receiving device discovery updates for ONE device.
 *
 * @param eui64
 * @param status
 */
void test_discovery_status_callback(zigbee_eui64_uint_t eui64,
                                    zigpc_discovery_status_t status);

/**
 * @brief Test callback for listening to all device discovery updates.
 *
 * @param eui64
 * @param status
 */
void test_discovery_status_listener(zigbee_eui64_uint_t eui64,
                                    zigpc_discovery_status_t status);

#endif  // TEST_ZIGPC_DISCOVERY_CALLBACKS_H
