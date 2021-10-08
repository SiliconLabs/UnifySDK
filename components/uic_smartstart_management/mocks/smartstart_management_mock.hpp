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
 * @file smartstart_management_mock.hpp
 * @brief TODO: Write brief for smartstart_management_mock
 *
 * TODO: Write component description for smartstart_management_mock
 *
 * @{
 */

#ifndef SMARTSTART_MANAGEMENT_MOCK_HPP
#define SMARTSTART_MANAGEMENT_MOCK_HPP

#include <string>
#include "smartstart_management.hpp"

extern std::string received_dsk;
extern std::string received_device_unid;

extern bool notify_node_added_called;
extern bool notify_node_removed_called;
extern bool set_manual_intervention_required_called;

extern sl_status_t notify_node_removed_return_value;
extern sl_status_t notify_node_added_return_value;
extern sl_status_t set_manual_intervention_return_value;

#endif  //SMARTSTART_MANAGEMENT_MOCK_HPP
/** @} end smartstart_management_mock */
