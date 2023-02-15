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

#include "config.h"
#include "process.h"
#include "sl_log.h"
#include "uic_mqtt.h"
#include "dotdot_mqtt.h"
#include "dotdot_mqtt_send_commands.h"
#include "ucl_mqtt_helper.h"
#include "positioning.h"
#include "aoa_loc.h"
#include "angle_queue.h"
#include "aoa_parse.h"
#include "dotdot_mqtt_attributes.h"
#include "positioning_configuration_handler.h"
#include "positioning_id.h"

#define LOG_TAG "positioning"

// Endpoint ID of the AoX Locator cluster, has to match with the  same setting
// in the aoxpc_unid component.
#define AOXPC_ENDPOINT_ID 0

// Configuration keys
#define CONFIG_KEY_POSITIONING_CONFIG_FILE      "positioning.config"
#define CONFIG_KEY_POSITIONING_ANGLE_CORRECTION "positioning.angle_correction"

static sl_status_t parse_config_file(void);
static void positioning_teardown(void);
static sl_status_t
  angle_report_callback(dotdot_unid_t unid,
                        dotdot_endpoint_id_t endpoint,
                        uic_mqtt_dotdot_callback_call_type_t call_type,
                        const char *tag_unid,
                        SphericalCoordinates direction,
                        SphericalCoordinates deviation,
                        int32_t sequence);
static void on_angles_ready(aoa_id_t tag_id,
                            uint32_t angle_count,
                            aoa_angle_t *angle_list,
                            aoa_id_t *locator_list);
static sl_status_t locator_position_and_orientation_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  bool unretained,
  uic_mqtt_dotdot_attribute_update_type_t update_type,
  CoordinateAndOrientation position);
static sl_status_t locator_position_and_orientation_valid_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  bool unretained,
  uic_mqtt_dotdot_attribute_update_type_t update_type,
  bool position_and_orientation_valid);

// Positioning process definition
PROCESS(positioning_process, "Positioning process");

// Positioning process implementation
PROCESS_THREAD(positioning_process, ev, data)
{
  PROCESS_BEGIN()
  while (1) {
    if (ev == PROCESS_EVENT_EXIT) {
      positioning_teardown();
    }
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}

sl_status_t positioning_config_init(void)
{
  config_status_t status
    = config_add_string(CONFIG_KEY_POSITIONING_CONFIG_FILE,
                        "Configuration file containing positioning parameters",
                        "");
  status
    |= config_add_bool(CONFIG_KEY_POSITIONING_ANGLE_CORRECTION,
                       "Enable/disable angle correction feedback feature",
                       true);

  if (status != CONFIG_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

sl_status_t positioning_fixt_setup(void)
{
  process_start(&positioning_process, NULL);
  uic_mqtt_dotdot_aox_locator_generated_angle_report_callback_set(angle_report_callback);
  uic_mqtt_dotdot_aox_locator_attribute_position_and_orientation_callback_set(
    &locator_position_and_orientation_callback);
  uic_mqtt_dotdot_aox_locator_attribute_position_and_orientation_valid_callback_set(
    &locator_position_and_orientation_valid_callback);
  angle_queue_config.on_angles_ready = on_angles_ready;
  return parse_config_file();
}

static sl_status_t parse_config_file(void)
{
  sl_status_t sc = SL_STATUS_OK;
  const char *config_file = NULL;

  if (CONFIG_STATUS_OK
      != config_get_as_string(CONFIG_KEY_POSITIONING_CONFIG_FILE,
                              &config_file)) {
    sl_log_error(LOG_TAG, "Failed to get positioning.config");
    sc = SL_STATUS_FAIL;
  }

  if ((config_file != NULL) && (strlen(config_file) > 0)) {
    positioning_configuration_handler_parse_config_file(config_file);
  }

  return sc;
}

static sl_status_t
  angle_report_callback(dotdot_unid_t unid,
                        dotdot_endpoint_id_t endpoint,
                        uic_mqtt_dotdot_callback_call_type_t call_type,
                        const char *tag_unid,
                        SphericalCoordinates direction,
                        SphericalCoordinates deviation,
                        int32_t sequence)
{
  (void)deviation;  // Not supported yet.
  (void)endpoint;

  sl_status_t sc;
  aoa_locator_t *locator;
  aoa_asset_tag_t *tag;
  aoa_angle_t angle;

  // If dotdot MQTT asks if this command is supported, we say no, because we only
  // want to read the data from others.
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Find locator.
  sc = aoa_loc_get_locator_by_id((char *)unid, NULL, &locator);
  if (sc != SL_STATUS_OK) {
    sl_log_error(LOG_TAG, "Unknown locator: %s", unid);
    return SL_STATUS_OK;
  }

  // Find asset tag.
  sc = aoa_loc_get_tag_by_id((char *)tag_unid, &tag);
  if (SL_STATUS_NOT_FOUND == sc) {
    // Add new tag
    sc = aoa_loc_add_asset_tag((char *)tag_unid, &tag);
    if (sc != SL_STATUS_OK) {
      sl_log_error(LOG_TAG, "aoa_loc_add_asset_tag failed: 0x%04x", sc);
      return sc;
    }

    publish_tag_state((char *)tag_unid);

    sl_log_info(LOG_TAG, "New tag added : %s \n", tag_unid);
  }

  // Convert callback data into internal angle structure.
  angle.azimuth   = direction.Azimuth;
  angle.elevation = direction.Elevation;
  angle.distance  = direction.Distance;
  angle.sequence = sequence;

  sc = angle_queue_push((char *)tag_unid,
                        (char *)unid,
                        &angle);
  if (sc == SL_STATUS_NOT_INITIALIZED) {
    sl_log_debug(LOG_TAG,
                 "Angle queue is not initialized. Angle data for locator %s / "
                 "asset tag %s will not be processed.",
                 unid,
                 tag_unid);
  }
  else if (sc != SL_STATUS_OK) {
    sl_log_error(LOG_TAG, "angle_queue_push failed: 0x%04x", sc);
  }

  return sc;
}

/**
 * @brief Process AoXLocator position/orientation updates
 *
 * @param unid          refer to dotdot_mqtt_attributes.h for fields description
 * @param endpoint      refer to dotdot_mqtt_attributes.h for fields description
 * @param unretained    refer to dotdot_mqtt_attributes.h for fields description
 * @param update_type   refer to dotdot_mqtt_attributes.h for fields description
 * @param position      refer to dotdot_mqtt_attributes.h for fields description
 * @return sl_status_t
 */
static sl_status_t locator_position_and_orientation_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  bool unretained,
  uic_mqtt_dotdot_attribute_update_type_t update_type,
  CoordinateAndOrientation position)
{
  // Ignore updates of the Desired value.
  if (UCL_DESIRED_UPDATED == update_type) {
    return SL_STATUS_OK;
  }
  sl_status_t sc;
  aoa_locator_t *locator;
  struct sl_rtl_loc_locator_item item      = {};

  // Delete every tag.
  // Tags will be added when new data arrives.
  aoa_loc_destroy_tags();
  angle_queue_deinit();

  // Reinitialize the estimator.
  sc = aoa_loc_reinit();
  if (sc != SL_STATUS_OK) {
    return sc;
  }

  // Find locator.
  sc = aoa_loc_get_locator_by_id((char *)unid, NULL, &locator);
  if (sc != SL_STATUS_OK) {
    // New locator.
    if (!unretained) {
      item.coordinate_x               = position.CoordinateX;
      item.coordinate_y               = position.CoordinateY;
      item.coordinate_z               = position.CoordinateZ;
      item.orientation_x_axis_degrees = position.OrientationX;
      item.orientation_y_axis_degrees = position.OrientationY;
      item.orientation_z_axis_degrees = position.OrientationZ;

      sc = aoa_loc_add_locator((char *)unid, item, &locator);
      if (sc != SL_STATUS_OK) {
        return sc;
      }
      sl_log_info(LOG_TAG, "Locator %s added.", unid);
    }
  } else {
    // Known locator.
    if (unretained) {
      // Locator has to be removed.
      sc = aoa_loc_remove_locator((char *)unid);
      if (sc != SL_STATUS_OK) {
        return sc;
      }
      sl_log_info(LOG_TAG, "Locator %s removed.", unid);
    } else {
      locator->item.coordinate_x               = position.CoordinateX;
      locator->item.coordinate_y               = position.CoordinateY;
      locator->item.coordinate_z               = position.CoordinateZ;
      locator->item.orientation_x_axis_degrees = position.OrientationX;
      locator->item.orientation_y_axis_degrees = position.OrientationY;
      locator->item.orientation_z_axis_degrees = position.OrientationZ;
    }
  }

  sc = aoa_loc_finalize_config();
  if (sc != SL_STATUS_OK) {
    return sc;
  }

  angle_queue_config.locator_count = aoa_loc_get_number_of_locators();

  sc = angle_queue_init(&angle_queue_config);
  if (sc != SL_STATUS_OK) {
    return sc;
  }

  return SL_STATUS_OK;
}

static sl_status_t locator_position_and_orientation_valid_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  bool unretained,
  uic_mqtt_dotdot_attribute_update_type_t update_type,
  bool position_and_orientation_valid)
{
  sl_status_t sc;
  aoa_locator_t *locator = NULL;
  struct sl_rtl_loc_locator_item item = {};
  uint32_t locator_idx;

  if (UCL_DESIRED_UPDATED == update_type) {
    return SL_STATUS_OK;
  }

  // Delete every tag.
  // Tags will be added when new data arrives.
  aoa_loc_destroy_tags();
  angle_queue_deinit();

  // Reinitialize the estimator.
  sc = aoa_loc_reinit();
  if (sc != SL_STATUS_OK) {
    return sc;
  }

  // Find locator.
  sc = aoa_loc_get_locator_by_id((char *)unid, &locator_idx, &locator);
  if (sc != SL_STATUS_OK) {
    // New locator.
    if (!unretained) {
      sc = aoa_loc_add_locator((char *)unid, item, &locator);
      if (sc != SL_STATUS_OK) {
        return sc;
      }
      sl_log_info(LOG_TAG, "Locator %s added.", unid);
    }
  } else {
    // Known locator.
    if (unretained) {
      // Locator has to be removed.
      sc = aoa_loc_remove_locator((char *)unid);
      if (sc != SL_STATUS_OK) {
        return sc;
      }
      sl_log_info(LOG_TAG, "Locator %s removed.", unid);
    }
  }

  if(NULL != locator) {
    locator->functional = position_and_orientation_valid;
  }

  sc = aoa_loc_finalize_config();
  if (sc != SL_STATUS_OK) {
    return sc;
  }

  angle_queue_config.locator_count = aoa_loc_get_number_of_locators();

  sc = angle_queue_init(&angle_queue_config);
  if (sc != SL_STATUS_OK) {
    return sc;
  }

  return SL_STATUS_OK;
}

static void positioning_teardown(void)
{
  // Remove all retained topics.
  uic_mqtt_unretain_by_regex(REGEX_ALL_TOPICS);
}

/**************************************************************************//**
 * Angles ready callback for angle queue.
 *****************************************************************************/
static void on_angles_ready(aoa_id_t tag_id,
                            uint32_t angle_count,
                            aoa_angle_t *angle_list,
                            aoa_id_t *locator_list)
{
  sl_status_t sc;

  sc = aoa_loc_calc_position(tag_id,
                             angle_count,
                             angle_list,
                             locator_list);
  if (SL_STATUS_OK != sc) {
    sl_log_error(LOG_TAG, "Position calculation failed: 0x%04x", sc);
  }
}

/**************************************************************************//**
 * Position ready callback.
 *****************************************************************************/
void aoa_loc_on_position_ready(aoa_asset_tag_t *tag)
{
  // Always use endpoint 0 for publishing the position.
  const char base_topic_template[] = "ucl/by-unid/%s/ep0";
  char base_topic[sizeof(base_topic_template) + sizeof(aoa_id_t)];

  EstimatedPosition value = {
    .X = tag->position.x,
    .Y = tag->position.y,
    .Z = tag->position.z,
    .DeviationX = tag->position.x_stdev,
    .DeviationY = tag->position.y_stdev,
    .DeviationZ = tag->position.z_stdev,
    .Sequence = tag->position.sequence,
    .ApplicationId = NULL,
  };

  value.ApplicationId = (char *)get_positioning_id();

  snprintf(base_topic, sizeof(base_topic), base_topic_template, tag->id);

  (void)uic_mqtt_dotdot_aox_position_estimation_position_publish(
    base_topic,
    value,
    UCL_MQTT_PUBLISH_TYPE_ALL);
}

/**************************************************************************//**
 * Correction ready callback.
 *****************************************************************************/
void aoa_loc_on_correction_ready(aoa_asset_tag_t *tag,
                                 int32_t sequence,
                                 aoa_id_t loc_id,
                                 uint32_t loc_idx,
                                 aoa_angle_t *correction)
{
  bool enabled = true;
  config_get_as_bool(CONFIG_KEY_POSITIONING_ANGLE_CORRECTION, &enabled);
  if (!enabled) {
    // Angle correction feedback is disabled.
    return;
  }

  const uic_mqtt_dotdot_aox_locator_command_angle_correction_fields_t fields
    = {.direction.Azimuth   = correction->azimuth,
       .direction.Elevation = correction->elevation,
       .direction.Distance  = correction->distance,
       .deviation.Azimuth   = correction->azimuth_stdev,
       .deviation.Elevation = correction->elevation_stdev,
       .deviation.Distance  = correction->distance_stdev,
       .sequence            = correction->sequence,
       .tag_unid            = tag->id};

  uic_mqtt_dotdot_aox_locator_publish_angle_correction_command(
    (dotdot_unid_t)loc_id,
    AOXPC_ENDPOINT_ID,
    &fields);
}
