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
#include "aoxpc_configuration_handler.h"

// AoXPC
#include "aoxpc_unid.h"
#include "aox_locator_configuration.h"

// Unify components
#include "dotdot_mqtt_supported_generated_commands.h"

// Gecko SDK
#include "aoa_db.h"
#include "aoa_parse.h"
#include "aoa_cte_parse.h"

#ifdef AOA_ANGLE
#include "aoa_angle.h"
#endif  // AOA_ANGLE

// Generic includes
#include <stdlib.h>

// Unify components
#include "sl_log.h"

// NCP
#include "ncp.h"

#define LOG_TAG "aoxpc_configuration_handler"

// Contants
#define MAXIMUM_ALLOW_LIST_SIZE     5000
#define MAXIMUM_AZIMUTH_MASK_SIZE   MAXIMUM_ALLOW_LIST_SIZE
#define MAXIMUM_ELEVATION_MASK_SIZE MAXIMUM_ALLOW_LIST_SIZE

#ifdef AOA_ANGLE
// AoX Mode strings
static const char *aox_mode_string[] = {
  "PLACEHOLDER",
  "PLACEHOLDER",
  "PLACEHOLDER",
  "SL_RTL_AOX_MODE_ONE_SHOT_BASIC",
  "SL_RTL_AOX_MODE_ONE_SHOT_BASIC_LIGHTWEIGHT",
  "SL_RTL_AOX_MODE_ONE_SHOT_FAST_RESPONSE",
  "SL_RTL_AOX_MODE_ONE_SHOT_HIGH_ACCURACY",
  "SL_RTL_AOX_MODE_ONE_SHOT_BASIC_AZIMUTH_ONLY",
  "SL_RTL_AOX_MODE_ONE_SHOT_FAST_RESPONSE_AZIMUTH_ONLY",
  "SL_RTL_AOX_MODE_ONE_SHOT_HIGH_ACCURACY_AZIMUTH_ONLY",
  "SL_RTL_AOX_MODE_REAL_TIME_FAST_RESPONSE",
  "SL_RTL_AOX_MODE_REAL_TIME_BASIC",
  "SL_RTL_AOX_MODE_REAL_TIME_HIGH_ACCURACY",
};

// AoX antenna array type strings
static const char *antenna_type_string[] = {
  "SL_RTL_AOX_ARRAY_TYPE_4x4_URA",
  "SL_RTL_AOX_ARRAY_TYPE_3x3_URA",
  "SL_RTL_AOX_ARRAY_TYPE_1x4_ULA",
  "SL_RTL_AOX_ARRAY_TYPE_4x4_DP_URA",
  "SL_RTL_AOX_ARRAY_TYPE_COREHW_15x15_DP",
  "SL_RTL_AOX_ARRAY_TYPE_COREHW_12x12_DP",
};
#endif

///////////////////////////////////////////////////////////////////////////////
// Private functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Parses the content of the AoXPC Configuration file
 *
 * @param config buffer with the contents of the configuration file.
 */
static void aoxpc_configuration_handler_parse_config(const char *config)
{
  // Fetch the AoXPC UNID
  aoa_id_t locator_unid = {};
  aoa_id_t temp_locator = {};

  if (SL_STATUS_OK != get_aoxpc_unid(locator_unid)) {
    sl_log_error(LOG_TAG,
                 "Could not retrieve AoXPC UNID. "
                 "FIXME: Parsing the configuration should not need this.");
  }

  aoa_id_t allow_id;
  sl_status_t sc;
  uint8_t address[ADR_LEN];
  uint8_t address_type;
  aoa_cte_type_t cte_mode;
  struct sl_rtl_loc_locator_item item;
  CoordinateAndOrientation position = {};
  uint8_t *antenna_switch_pattern = NULL;
  uint8_t antenna_switch_pattern_size = 0;
  enum sl_rtl_aox_array_type antenna_array_type;

#ifdef AOA_ANGLE
  float mask_min                   = 0;
  float mask_max                   = 0;
  aoa_angle_config_t *angle_config = NULL;

  sc = aoa_angle_get_config(locator_unid, &angle_config);
  if (SL_STATUS_OK != sc) {
    sl_log_error(LOG_TAG, "Failed to get angle config.");
  }
#endif

  sc = aoa_parse_init(config);
  if (SL_STATUS_OK != sc) {
    sl_log_error(LOG_TAG, "Failed to parse config.");
    return;
  }

  sl_log_info(LOG_TAG, "Start parsing configuration...");

   while (aoa_parse_locator(temp_locator, &item) == SL_STATUS_OK) {
      if (aoa_id_compare(temp_locator, locator_unid) == 0) {
        position.CoordinateX  = item.coordinate_x;
        position.CoordinateY  = item.coordinate_y;
        position.CoordinateZ  = item.coordinate_z;
        position.OrientationX = item.orientation_x_axis_degrees;
        position.OrientationY = item.orientation_y_axis_degrees;
        position.OrientationZ = item.orientation_z_axis_degrees;
        aox_locator_set_position_and_orientation_attribute(position);

        sl_log_info(LOG_TAG,
                    "coordinate: %f %f %f",
                    item.coordinate_x,
                    item.coordinate_y,
                    item.coordinate_z);
        sl_log_info(LOG_TAG,
                    "orientation: %f %f %f",
                    item.orientation_x_axis_degrees,
                    item.orientation_y_axis_degrees,
                    item.orientation_z_axis_degrees);

        aox_locator_set_position_and_orientation_valid_attribute(true);
        break;
        }
    }

#ifdef AOA_ANGLE
  sc = aoa_parse_aox_mode(&angle_config->aox_mode, locator_unid);
  if (sc == SL_STATUS_OK) {
    sl_log_info(LOG_TAG,
                "AoX mode set to: %s",
                aox_mode_string[angle_config->aox_mode]);
    // TBD: AoX Mode is not supported in MQTT; do nothing?
  }
#endif

  sc = aoa_parse_antenna_mode(&antenna_array_type, locator_unid);
  if (sc == SL_STATUS_OK) {
    sl_log_info(LOG_TAG,
                "Antenna mode set to: %s",
                antenna_type_string[antenna_array_type]);
    // TBD: Antenna Mode is not supported in MQTT; do nothing?
#ifdef AOA_ANGLE
    sc = antenna_array_init(&angle_config->antenna_array, antenna_array_type);
    if (sc != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   "antenna_array_init failed for %s",
                    antenna_type_string[antenna_array_type]);
    }
#endif
  }

  sc = aoa_parse_antenna_array(&antenna_switch_pattern,
                               &antenna_switch_pattern_size,
                               locator_unid);

  if (sc == SL_STATUS_OK) {
    sl_log_info(LOG_TAG, "Antenna array set to: ");
    for (uint8_t i = 0; i < antenna_switch_pattern_size; i++) {
      sl_log_info(LOG_TAG, "%d,", antenna_switch_pattern[i]);
    }
    // TBD: Antenna array is not supported in MQTT; do nothing?
#ifdef ANGLE
    sc = antenna_array_set_pattern(&angle_config->antenna_array,
                                  antenna_switch_pattern,
                                  antenna_switch_pattern_size);
    if (sc != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   "antenna_array_set_pattern failed with size %d",
                   antenna_switch_pattern_size);
    }
#endif
    free(antenna_switch_pattern);
  }

#ifdef AOA_ANGLE
  sc = aoa_parse_angle_filtering(&angle_config->angle_filtering, locator_unid);
  if (sc == SL_STATUS_OK) {
    sl_log_info(LOG_TAG,
                "Angle filtering set to: %s",
                angle_config->angle_filtering ? "enabled" : "disabled");
    // TBD: Angle filtering is not supported in MQTT; do nothing?
  }

  sc = aoa_parse_angle_filtering_weight(&angle_config->angle_filtering_weight,
                                        locator_unid);
  if (sc == SL_STATUS_OK) {
    sl_log_info(LOG_TAG,
                "Angle filtering weight set to: %f",
                angle_config->angle_filtering_weight);
    // TBD: Angle filtering weight is not supported in MQTT; do nothing?
  }

  sc = aoa_parse_simple_config(&angle_config->angle_correction_timeout,
                               "angleCorrectionTimeout",
                               locator_unid);
  if (sc == SL_STATUS_OK) {
    sl_log_info(LOG_TAG,
                "Angle correction timeout set to: %d",
                angle_config->angle_correction_timeout);
    // TBD: Angle correction is not supported in MQTT; do nothing?
  }

  sc = aoa_parse_simple_config(&angle_config->angle_correction_delay,
                               "angleCorrectionDelay",
                               locator_unid);
  if (sc == SL_STATUS_OK) {
    sl_log_info(LOG_TAG,
                "Angle correction delay set to: %d",
                angle_config->angle_correction_delay);
    // TBD: Angle correction delay is not supported in MQTT; do nothing?
  }
#endif

  sc = aoa_cte_parse_mode(&cte_mode, locator_unid);
  if (sc == SL_STATUS_OK) {
    sl_log_info(LOG_TAG,
                "CTE mode set to: %s",
                cte_mode == AOA_CTE_TYPE_SILABS ? "SILABS"
                : cte_mode == AOA_CTE_TYPE_CONN ? "CONN"
                                                : "CONNLESS");
    // Tell AoXLocator MQTT about the CTE Mode setting
    aox_locator_set_cte_mode_attribute(cte_mode);
  }

  sc = aoa_parse_simple_config(&aoa_cte_config.cte_sampling_interval,
                               "cteSamplingInterval",
                               locator_unid);
  if (sc == SL_STATUS_OK) {
    sl_log_info(LOG_TAG,
                "CTE sampling interval set to: %d",
                aoa_cte_config.cte_sampling_interval);
    // TBD: CTE sample interval is not supported in MQTT; do nothing?
  }

  sc = aoa_parse_simple_config(&aoa_cte_config.cte_min_length,
                               "cteLength",
                               locator_unid);
  if (sc == SL_STATUS_OK) {
#ifdef AOA_ANGLE
    angle_config->cte_min_length = aoa_cte_config.cte_min_length;
#endif
    sl_log_info(LOG_TAG,
                "CTE length set to: %d",
                aoa_cte_config.cte_min_length);
    // TBD: CTE Length is not supported in MQTT; do nothing?
  }

  sc = aoa_parse_simple_config(&aoa_cte_config.cte_slot_duration,
                               "slotDuration",
                               locator_unid);
  if (sc == SL_STATUS_OK) {
#ifdef AOA_ANGLE
    angle_config->cte_slot_duration = aoa_cte_config.cte_slot_duration;
#endif
    sl_log_info(LOG_TAG,
                "CTE slot duration set to: %d",
                aoa_cte_config.cte_slot_duration);
    // TBD: CTE Slot duration is not supported in MQTT; do nothing?
  }

  if (SL_STATUS_OK == aoa_parse_check_config_exist("allowlist", locator_unid)) {
    char *allow_list[MAXIMUM_ALLOW_LIST_SIZE] = {};
    size_t allow_list_count                   = 0;
    do {
      sc = aoa_parse_allowlist(address, &address_type, locator_unid);
      if (sc == SL_STATUS_OK) {
        aoa_address_to_id(address, address_type, allow_id);
        sl_log_info(LOG_TAG, "Adding tag id '%s' to the allowlist.", allow_id);
        allow_list_count += 1;
        allow_list[allow_list_count - 1] = malloc(sizeof(aoa_id_t));
        aoa_id_copy(allow_list[allow_list_count - 1], allow_id);
      } else if (SL_STATUS_NOT_FOUND != sc) {
        sl_log_error(LOG_TAG, "Parsing allowlist failed.");
      }
    } while (sc == SL_STATUS_OK);

    // Tell AoXLocator MQTT about our new Allow List
    aox_locator_set_allow_list_attribute(allow_list_count,
                                         (const char **)allow_list);

    // Free the malloc'ed memory:
    for (size_t i = 0; i < allow_list_count; i++) {
      free(allow_list[i]);
    }
  }

#ifdef AOA_ANGLE
  if (SL_STATUS_OK
      == aoa_parse_check_config_exist("azimuthMask", locator_unid)) {
    MinMaxPair azimuth_mask[MAXIMUM_AZIMUTH_MASK_SIZE] = {};
    size_t azimuth_mask_count                          = 0;
    do {
      sc = aoa_parse_azimuth(&mask_min, &mask_max, locator_unid);
      if (sc == SL_STATUS_OK) {
        azimuth_mask_count += 1;
        azimuth_mask[azimuth_mask_count - 1].Min = mask_min;
        azimuth_mask[azimuth_mask_count - 1].Max = mask_max;
        sl_log_info(LOG_TAG,
                    "Azimuth mask added: min: %f, max: %f",
                    mask_min,
                    mask_max);
      }
    } while (sc == SL_STATUS_OK);

    // Tell AoXLocator MQTT about our new Azimuth Mask
    aox_locator_set_azimuth_mask_attribute(azimuth_mask_count, azimuth_mask);
  }

  if (SL_STATUS_OK
      == aoa_parse_check_config_exist("elevationMask", locator_unid)) {
    MinMaxPair elevation_mask[MAXIMUM_ELEVATION_MASK_SIZE] = {};
    size_t elevation_mask_count                            = 0;
    do {
      if (sc == SL_STATUS_OK) {
        sc = aoa_parse_elevation(&mask_min, &mask_max, locator_unid);
        if (SL_STATUS_OK == sc) {
          elevation_mask_count += 1;
          elevation_mask[elevation_mask_count - 1].Min = mask_min;
          elevation_mask[elevation_mask_count - 1].Max = mask_max;
          sl_log_info(LOG_TAG,
                      "Elevation mask added: min: %f, max: %f",
                      mask_min,
                      mask_max);
        }
      }
    } while (sc == SL_STATUS_OK);

    // Tell AoXLocator MQTT about our new Elevation Mask
    aox_locator_set_elevation_mask_attribute(elevation_mask_count,
                                             elevation_mask);
  }

  sc = aoa_angle_finalize_config(locator_unid);
  if (SL_STATUS_OK != sc) {
    sl_log_error(LOG_TAG, "Failed to finalize angle calculation config.");
  }
#endif

  sc = aoa_parse_deinit();
  if (SL_STATUS_OK != sc) {
    sl_log_error(LOG_TAG, "Failed to deinit config parser.");
  }

  // Notify that an update happened to AoXLocator / MQTT.
  aoxpc_configuration_handler_on_aox_locator_configuration_updated();
}

///////////////////////////////////////////////////////////////////////////////
// Public functions
///////////////////////////////////////////////////////////////////////////////
void aoxpc_configuration_handler_on_aox_locator_configuration_updated(void)
{
  // Fetch the AoXPC UNID
  aoa_id_t locator_unid = {};
  if (SL_STATUS_OK != get_aoxpc_unid(locator_unid)) {
    sl_log_error(LOG_TAG,
                 "Could not retrieve AoXPC UNID. "
                 "MQTT Configuration will not be propagated to the aox db");
    return;
  }

  sl_log_debug(LOG_TAG,
               "AoXLocator configuration has been updated. "
               "Propagating changes to the aoa database");

  const aox_locator_attributes_t *aox_locator_config
    = aox_locator_configuration_get_configuration();

  // Remove all tags from the db.
  // It will be reinitialized when
  // new data arrives.
  aoa_db_remove_all();

  // ReportingMode
  // This attribute is not supported, so for now we statically publish
  // that only AngleReports are generated
  uic_mqtt_dotdot_aox_locator_supported_commands_t commands
    = {.angle_report = true};
  uic_mqtt_dotdot_aox_locator_publish_supported_generated_commands(
    locator_unid,
    AOXPC_ENDPOINT_ID,
    &commands);

  // Configure the Allow List
  aoa_db_allowlist_reset();
  for (size_t i = 0; i < aox_locator_config->allow_list_count; i++) {
    // Get the address from the ID:
    uint8_t allowed_address[ADR_LEN] = {};
    uint8_t allowed_address_type     = 0;

    // Convert ID to address:
    if (SL_STATUS_OK
        != aoa_id_to_address(aox_locator_config->allow_list[i],
                             allowed_address,
                             &allowed_address_type)) {
      sl_log_error(LOG_TAG,
                   "Failed to retrieve address from ID %s. "
                   "It will not be part of the AllowList",
                   aox_locator_config->allow_list[i]);
      continue;
    }

    // Push it to the AoA db.
    // TBD: Address type is not used here?
    if (SL_STATUS_OK != aoa_db_allowlist_add(allowed_address)) {
      sl_log_error(LOG_TAG,
                   "Error adding ID %s to the Allow List. Aborting",
                   aox_locator_config->allow_list[i]);
      break;
    }
  }

  // Configure the Azimuth Masks
  aoa_angle_reset_azimuth_masks(locator_unid);
  for (size_t i = 0; i < aox_locator_config->azimuth_mask_count; i++) {
    if (SL_STATUS_OK
        != aoa_angle_add_azimuth_mask(
          locator_unid,
          aox_locator_config->azimuth_mask[i].Min,
          aox_locator_config->azimuth_mask[i].Max)) {
      sl_log_error(LOG_TAG, "Adding azimuth mask %i failed.", i);
    }
  }

  // Configure the Elevation Masks
  aoa_angle_reset_elevation_masks(locator_unid);
  for (size_t i = 0; i < aox_locator_config->elevation_mask_count; i++) {
    if (SL_STATUS_OK
        != aoa_angle_add_elevation_mask(
          locator_unid,
          aox_locator_config->elevation_mask[i].Min,
          aox_locator_config->elevation_mask[i].Max)) {
      sl_log_error(LOG_TAG, "Adding elevation mask %i failed.", i);
    }
  }

  // Configure the CTE mode
  aoa_cte_set_mode(aox_locator_config->cte_mode);

  // Finally, back up this new configuration in the Attribute Store
  aox_locator_store_attributes_in_attribute_store();

  // Reset the ncp target.
  ncp_reset();
}

void aoxpc_configuration_handler_parse_config_file(const char *file_name)
{
  char *buffer;
  buffer = aoa_parse_load_file(file_name);
  if (buffer == NULL) {
    sl_log_error(LOG_TAG, "Failed to load file: %s", file_name);
    sl_log_warning(LOG_TAG, "Continue with default settings");
  } else {
    aoxpc_configuration_handler_parse_config(buffer);
    free(buffer);
  }
}
