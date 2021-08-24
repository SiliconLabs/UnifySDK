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
// Includes from this component
#include "on_off_cluster_mapper.h"
#include "dotdot_attributes.h"

// Includes from UIC
#include "sl_log.h"
#include "sl_status.h"

// Includes from auto-generated files
#include "dotdot_mqtt.h"

// Setup Log ID
#define LOG_TAG "on_off_cluster_mapper"

constexpr int32_t ON_VALUE  = 0xFF;
constexpr int32_t OFF_VALUE = 0x00;

///////////////////////////////////////////////////////////////////////////////
// Forward declarations
///////////////////////////////////////////////////////////////////////////////
sl_status_t
  on_off_cluster_mapper_on(const dotdot_unid_t unid,
                           const dotdot_endpoint_id_t endpoint,
                           uic_mqtt_dotdot_callback_call_type_t callback_type);
sl_status_t
  on_off_cluster_mapper_off(const dotdot_unid_t unid,
                            const dotdot_endpoint_id_t endpoint,
                            uic_mqtt_dotdot_callback_call_type_t callback_type);
sl_status_t on_off_cluster_mapper_toggle(
  const dotdot_unid_t unid,
  const dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t callback_type);
///////////////////////////////////////////////////////////////////////////////
// Helper function
///////////////////////////////////////////////////////////////////////////////
static void set_on_off_cluster(const dotdot_unid_t unid,
                               const dotdot_endpoint_id_t endpoint,
                               int32_t value)
{
  if (dotdot_is_supported_on_off_on_off(unid, endpoint)) {
    dotdot_set_on_off_on_off(unid, endpoint, DESIRED_ATTRIBUTE, value);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Control functions
///////////////////////////////////////////////////////////////////////////////

sl_status_t
  on_off_cluster_mapper_on(const dotdot_unid_t unid,
                           const dotdot_endpoint_id_t endpoint,
                           uic_mqtt_dotdot_callback_call_type_t callback_type)
{
  if (callback_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return dotdot_is_supported_on_off_on_off(unid, endpoint) ? SL_STATUS_OK
                                                             : SL_STATUS_FAIL;
  }
  set_on_off_cluster(unid, endpoint, ON_VALUE);
  return SL_STATUS_OK;
}

sl_status_t
  on_off_cluster_mapper_off(const dotdot_unid_t unid,
                            const dotdot_endpoint_id_t endpoint,
                            uic_mqtt_dotdot_callback_call_type_t callback_type)
{
  if (callback_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return dotdot_is_supported_on_off_on_off(unid, endpoint) ? SL_STATUS_OK
                                                             : SL_STATUS_FAIL;
    ;
  }
  set_on_off_cluster(unid, endpoint, OFF_VALUE);
  return SL_STATUS_OK;
}

sl_status_t on_off_cluster_mapper_toggle(
  const dotdot_unid_t unid,
  const dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t callback_type)
{
  if (callback_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return dotdot_is_supported_on_off_on_off(unid, endpoint) ? SL_STATUS_OK
                                                             : SL_STATUS_FAIL;
  }
  int32_t set_value
    = (dotdot_get_on_off_on_off(unid, endpoint, REPORTED_ATTRIBUTE) == ON_VALUE)
        ? OFF_VALUE
        : ON_VALUE;

  dotdot_set_on_off_on_off(unid, endpoint, DESIRED_ATTRIBUTE, set_value);
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Init and teardown functions
///////////////////////////////////////////////////////////////////////////////
bool on_off_cluster_mapper_init()
{
  sl_log_debug(LOG_TAG, "OnOff cluster mapper initialization");

  uic_mqtt_dotdot_on_off_on_callback_set(on_off_cluster_mapper_on);
  uic_mqtt_dotdot_on_off_off_callback_set(on_off_cluster_mapper_off);
  uic_mqtt_dotdot_on_off_toggle_callback_set(on_off_cluster_mapper_toggle);

  return true;
}

int on_off_cluster_mapper_teardown()
{
  sl_log_debug(LOG_TAG, "Teardown of the OnOff cluster mapper");
  return 0;
}
