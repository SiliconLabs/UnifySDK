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
#include "zwave_command_class_manufacturer_specific_control.h"

#include "zwave_command_class_version_types.h"
#include "attribute_resolver.h"
#include "zwave_command_classes_utils.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute.hpp"
#include "attribute_timeouts.h"
#include "zwave_utils.h"
#include "sl_log.h"

#include "zwave_COMMAND_CLASS_MANUFACTURER_SPECIFIC_attribute_id.h"
#include "zwave_COMMAND_CLASS_MANUFACTURER_SPECIFIC_handlers.h"

using namespace attribute_store;

typedef uint8_t device_id_type_t;

sl_status_t zwave_DEVICE_SPECIFIC_REPORT_handle_report_command_override(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame,
  uint16_t frame_length)
{
  attribute endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  // If there is a type id 0, replace this with what ever type
  // this frame has
  attribute id_type = endpoint_node.child_by_type_and_value<uint8_t>(
    ATTRIBUTE_DEVICE_SPECIFIC_DEVICE_ID_TYPE,
    0);
  if (id_type.is_valid()) {
    id_type.set_reported<uint8_t>(frame[2] & 7);
  }
  return SL_STATUS_IDLE;
}

static void on_version_attribute_update(attribute_store_node_t _updated_node,
                                        attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }
  /*
   * For discoverying the device type id we start by using type id 0
   */
  attribute endpoint_node = attribute(_updated_node).parent();
  if (!endpoint_node.child_by_type(ATTRIBUTE_DEVICE_SPECIFIC_DEVICE_ID_TYPE)
         .is_valid())
    zwave_COMMAND_CLASS_MANUFACTURER_SPECIFIC_add_DEVICE_SPECIFIC_DEVICE_ID_TYPE(
      endpoint_node,
      0);
}

sl_status_t zwave_command_class_manufacturer_specific_control_init()
{
  attribute_store_register_callback_by_type(
    on_version_attribute_update,
    ATTRIBUTE_COMMAND_CLASS_MANUFACTURER_SPECIFIC_VERSION);

  return zwave_COMMAND_CLASS_MANUFACTURER_SPECIFIC_init();
}
