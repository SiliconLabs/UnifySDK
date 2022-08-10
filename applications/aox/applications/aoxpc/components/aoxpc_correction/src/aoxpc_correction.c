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

#include <stdlib.h>

#include "sl_log.h"
#include "dotdot_mqtt.h"
#include "aoxpc_correction.h"
#include "aoxpc_unid.h"

#include "aoa_db.h"
#include "aoa_util.h"
#include "aoa_angle.h"

#define LOG_TAG "aoxpc_correction"

static sl_status_t
  apply_correction_callback(dotdot_unid_t unid,
                            dotdot_endpoint_id_t endpoint,
                            uic_mqtt_dotdot_callback_call_type_t call_type,
                            const char *tag_unid,
                            SphericalCoordinates direction,
                            SphericalCoordinates deviation,
                            int32_t sequence);

sl_status_t aoxpc_correction_init(void)
{
  uic_mqtt_dotdot_aox_locator_angle_correction_callback_set(
    apply_correction_callback);
  return SL_STATUS_OK;
}

static sl_status_t
  apply_correction_callback(dotdot_unid_t unid,
                            dotdot_endpoint_id_t endpoint,
                            uic_mqtt_dotdot_callback_call_type_t call_type,
                            const char *tag_unid,
                            SphericalCoordinates direction,
                            SphericalCoordinates deviation,
                            int32_t sequence)
{
  aoa_angle_t correction;
  bd_addr tag_addr;
  uint8_t tag_addr_type;
  aoa_db_entry_t *tag = NULL;
  enum sl_rtl_error_code ec;
  sl_status_t sc;
  aoa_angle_config_t *angle_config = NULL;

  // If dotdot MQTT asks if this command is supported, we say yes.
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return SL_STATUS_OK;
  }

  // Check for locator UNID.
  if (!is_aoxpc_unid(unid) || (endpoint != AOXPC_ENDPOINT_ID)) {
    return SL_STATUS_NOT_FOUND;
  }

  sc = aoa_angle_get_config((char *)unid, &angle_config);
  if (SL_STATUS_OK != sc) {
    sl_log_error(LOG_TAG, "aoa_angle_get_config failed: 0x%04x", sc);
    return sc;
  }

  // Find asset tag in the database
  sc = aoa_id_to_address((char *)tag_unid, tag_addr.addr, &tag_addr_type);
  if (SL_STATUS_OK != sc) {
    sl_log_error(LOG_TAG, "aoa_id_to_address failed: 0x%04x", sc);
    return sc;
  }
  sc = aoa_db_get_tag_by_address(&tag_addr, &tag);
  if (SL_STATUS_OK != sc) {
    sl_log_error(LOG_TAG, "aoa_db_get_tag_by_address failed: 0x%04x", sc);
    return sc;
  }

  // Fill up internal correction structure.
  correction.azimuth   = direction.Azimuth;
  correction.elevation = direction.Elevation;
  correction.distance  = direction.Distance;
  correction.azimuth_stdev   = deviation.Azimuth;
  correction.elevation_stdev = deviation.Elevation;
  correction.distance_stdev  = deviation.Distance;
  correction.sequence = sequence;

  if (aoa_sequence_compare(tag->sequence, correction.sequence)
      <= angle_config->angle_correction_delay) {
    sl_log_info(LOG_TAG, "Apply correction #%d for asset tag '%s'",
                correction.sequence,
                tag_unid);
    ec = aoa_set_correction((aoa_state_t *)tag->user_data,
                            &correction,
                            (char *)unid);
    if (ec != SL_RTL_ERROR_SUCCESS) {
      sl_log_error(LOG_TAG, "aoa_set_correction failed: %d", ec);
      return SL_STATUS_FAIL;
    }
  } else {
    sl_log_info(LOG_TAG, "Omit correction #%d for asset tag '%s'",
                correction.sequence,
                tag_unid);
  }
  return SL_STATUS_OK;
}
