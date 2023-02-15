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
 * @file zigpc_smartstart_int.hpp
 * @defgroup zigpc_smartstart_int ZigPC SmartStart internal API
 * @ingroup zigpc_smartstart
 * @brief internal API for ZigPC SmartStart
 *
 * @{
 */

#ifndef ZIGPC_SMARTSTART_INT_HPP
#define ZIGPC_SMARTSTART_INT_HPP

#include <string>

#include "sl_status.h"
#include "zigpc_common_zigbee.h"
#include "zigpc_common_dsk.hpp"

extern const char LOG_TAG[];

/**
 * @brief Callback invokved when the smartstart list has been updated.
 *
 * @param entries_pending_inclusion   Bool value indicating if the list
 *                                    contains some entries pending inclusion.
 */
void zigpc_smartstart_on_list_update(bool entries_pending_inclusion);

/**
 * @brief Callback invoked when the network has initialized.
 *
 * @param event_data network initialization data
 */
void zigpc_smartstart_on_network_init(void *event_data);

/**
 * @brief Callback invoked when node addition has completed.
 *
 * @param event_data node add data
 */
void zigpc_smartstart_on_node_added(void *event_data);

/**
 * @brief Callback invoked when node removal has completed.
 *
 * @param event_data node remove data
 */
void zigpc_smartstart_on_node_removed(void *event_data);

/**
 * @brief Setup function to initialize observers.
 *
 */
sl_status_t zigpc_smartstart_init_observers(void);

/**
 * @brief Get UNID from EUI64 representation.
 *
 * @param eui64
 * @return std::string UNID string if successful, empty otherwise.
 */
std::string zigpc_smartstart_get_unid(zigbee_eui64_t eui64);

/**
 * @brief Parse a SmartStart DSK in the form of Z3 Install Code configuration.
 *
 * @param dsk         DSK string to parse
 * @param dsk_content DSK content to populate.
 * @return sl_status_t SL_STATUS_OK if parsing is successful,
 * SL_STATUS_FAIL otherwise.
 */
sl_status_t zigpc_smartstart_dsk_parse_install_code(
  const std::string &dsk, zigpc_common::dsk_install_code_t &dsk_content);

/** @} end zigpc_smartstart_int */

#endif /* ZIGPC_SMARTSTART_INT_HPP */
