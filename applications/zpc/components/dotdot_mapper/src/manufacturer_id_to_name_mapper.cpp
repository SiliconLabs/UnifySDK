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
#include "manufacturer_id_to_name_mapper.h"

// Includes from UIC
#include "sl_log.h"
#include "sl_status.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute_store_helper.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute.hpp"
#include "dotdot_attributes.h"

// Generic includes
#include <assert.h>
#include <string>
#include <map>
#include <vector>

// Setup Log ID
constexpr char LOG_TAG[] = "manufacturer_id_to_name_mapper";

#include "manufacturer_id_name_list.inc"

static void
  manufacturer_id_attribute_update(attribute_store_node_t updated_node,
                                   attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  attribute_store::attribute manufacturer_id_node(updated_node);
  // Get the Customer name from the lookup table
  std::string manufacturer_name = "Unknown Manufacturer";
  try {
    manufacturer_name
      = manufacturer_id_name_map.at(manufacturer_id_node.reported<uint32_t>());
  } catch (const std::exception &e) {
    sl_log_debug(LOG_TAG,
                 "Cannot read Manufacturer ID for node %d. "
                 "Assuming Unknown Manufacturer name.\n",
                 manufacturer_id_node);
  }
  attribute_store::attribute end_point_node
    = manufacturer_id_node.first_parent(ATTRIBUTE_ENDPOINT_ID);
  attribute_store::attribute dotdot_manf_node
    = end_point_node.emplace_node(DOTDOT_ATTRIBUTE_ID_BASIC_MANUFACTURER_NAME);

  attribute_store_set_reported_string(dotdot_manf_node,
                                      manufacturer_name.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// Init and teardown functions
///////////////////////////////////////////////////////////////////////////////
bool manufacturer_id_to_name_mapper_init()
{
  sl_log_debug(LOG_TAG, "Manufacturer ID to Name Mapper initialization");

  // Register ATTRIBUTE_MANUFACTURER_SPECIFIC_MANUFACTURER_ID update
  attribute_store_register_callback_by_type_and_state(
    manufacturer_id_attribute_update,
    ATTRIBUTE_MANUFACTURER_SPECIFIC_MANUFACTURER_ID,
    REPORTED_ATTRIBUTE);

  return true;
}

int manufacturer_id_to_name_mapper_teardown()
{
  sl_log_debug(LOG_TAG, "Teardown of the Manufacturer ID to Name Mapper");
  return 0;
}
