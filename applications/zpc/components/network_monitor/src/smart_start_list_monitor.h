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
 * @defgroup network_monitor_smart_start Network Monitor SmartStart list.
 * @ingroup network_monitor
 *
 * @brief Monitors network events and updates the SmartStart list accordingly
 *
 * The SmartStart list monitor is in charge of pushing updates to the SmartStart
 * list whenever something has changed in the network, it could be that a node
 * was manually included and we match its DSK from the list, or that the
 * ZPC is going through reset and we clear-up the UNIDs from the list.
 *
 * @{
 */

#ifndef SMART_START_LIST_MONITOR_H
#define SMART_START_LIST_MONITOR_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Intitializes the SmartStart list monitor.
 *
 * @returns sl_status_t code.
 */
sl_status_t smart_start_list_monitor_init();

#ifdef __cplusplus
}
#endif

#endif  //SMART_START_LIST_MONITOR_H
/** @} end smart_start_list_monitor */
