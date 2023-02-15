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
#include "unify_dotdot_attribute_store.h"
#include "unify_dotdot_attribute_store_registration.h"
#include "unify_dotdot_attribute_store_command_callbacks.h"
#include "unify_dotdot_attribute_store_write_attributes_command_callbacks.h"
#include "unify_dotdot_attribute_store_force_read_attributes_command_callbacks.h"
#include "unify_dotdot_attribute_store_attribute_publisher.h"

// Private variable avoiding to try to register duplicate attributes to the attribute store.
static bool attribute_store_registration_done = false;

sl_status_t unify_dotdot_attribute_store_init()
{
  sl_status_t init_status = SL_STATUS_OK;

  // Register all the ZCL/ DotDot attribtue to the Attribute Store.
  if (false == attribute_store_registration_done) {
    init_status |= unify_dotdot_attribute_store_registration_init();
    attribute_store_registration_done = true;
  }

  // Register command callbacks to DotDot MQTT.
  init_status |= unify_dotdot_attribute_store_command_callbacks_init();

  // Register auto-generated ForceReadAttribute handlers:
  init_status
    |= unify_dotdot_attribute_store_force_read_attributes_command_callbacks_init();

  // Register auto-generated WriteAttributes handlers:
  init_status
    |= unify_dotdot_attribute_store_write_attributes_command_callbacks_init();

  // Register callbacks for attribute publications
  init_status |= unify_dotdot_attribute_store_attribute_publisher_init();

  return init_status;
}
