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

#ifndef ZIGPC_DATASTORE_TEST_HPP
#define ZIGPC_DATASTORE_TEST_HPP

/* This is a workaround to hide the C++ includes from
 * Unity's generate_test_runner.rb
 */

#ifdef __cplusplus
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "zigpc_datastore.hpp"

namespace bpt = boost::property_tree;

#endif
#endif  // ZIGPC_DATASTORE_TEST_HPP
