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
#include "zpc_database_helper.hpp"

// ZPC includes
#include "attribute_store_defined_attribute_types.h"
using namespace attribute_store;

// Local cache of the endpoint list.
static std::vector<attribute> endpoint_list;

std::vector<attribute> endpoint_id_list()
{
  // If we already computed it, return the previous list
  if (endpoint_list.size() > 0) {
    return endpoint_list;
  }

  // Else compute it for the first time:
  attribute root = attribute::root();
  for (attribute home_id: root.children(ATTRIBUTE_HOME_ID)) {
    for (attribute node_id: home_id.children(ATTRIBUTE_NODE_ID)) {
      for (attribute endpoint_id: node_id.children(ATTRIBUTE_ENDPOINT_ID)) {
        endpoint_list.push_back(endpoint_id);
      }
    }
  }
  return endpoint_list;
}

std::vector<attribute> get_attribute_list(attribute_store_type_t attribute_type) {
  std::vector<attribute> attribute_list;

  auto endpoint_list = endpoint_id_list();

  for(auto endpoint: endpoint_list) {
    auto attributes = endpoint.children(attribute_type);
    attribute_list.insert(attribute_list.end(), attributes.begin(), attributes.end());
  }

  return attribute_list;
}