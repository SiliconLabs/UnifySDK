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
#include "identify_cluster_mapper.h"
#include "dotdot_attributes.h"

// Includes from UIC
#include "sl_log.h"
#include "sl_status.h"

// Includes from auto-generated files
#include "dotdot_mqtt.h"

static sl_status_t on_identify(dotdot_unid_t unid,
                               dotdot_endpoint_id_t endpoint,
                               uic_mqtt_dotdot_callback_call_type_t call_type,
                               uint16_t identify_time)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return dotdot_is_supported_identify_identify_time(unid, endpoint)
             ? SL_STATUS_OK
             : SL_STATUS_FAIL;
  }

  ;
  return dotdot_set_identify_identify_time(unid,
                                           endpoint,
                                           DESIRED_ATTRIBUTE,
                                           identify_time)
           ? SL_STATUS_OK
           : SL_STATUS_FAIL;
}

static sl_status_t on_query(dotdot_unid_t unid,
                            dotdot_endpoint_id_t endpoint,
                            uic_mqtt_dotdot_callback_call_type_t call_type)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return dotdot_is_supported_identify_identify_time(unid, endpoint)
             ? SL_STATUS_OK
             : SL_STATUS_FAIL;
  }

  // NOP

  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Init and teardown functions
///////////////////////////////////////////////////////////////////////////////
bool identify_cluster_mapper_init()
{
  uic_mqtt_dotdot_identify_identify_callback_set(on_identify);
  uic_mqtt_dotdot_identify_identify_query_callback_set(on_query);
  return true;
}

int identify_cluster_mapper_teardown()
{
  return 0;
}
