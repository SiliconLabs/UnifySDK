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
#include "positioning_configuration_handler.h"

// Generic includes
#include <stddef.h>

// Gecko SDK
#include "aoa_parse.h"
#include "aoa_parse_enum.h"
#include "aoa_loc.h"

// Unify components
#include "sl_log.h"

// Positioning unid
#include "positioning_id.h"

#define LOG_TAG "positioning_configuration_handler"

angle_queue_config_t angle_queue_config = ANGLE_QUEUE_DEFAULT_CONFIG;

///////////////////////////////////////////////////////////////////////////////
// Private functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Parses the content of the Positioning Configuration file
 *
 * @param config buffer with the contents of the configuration file.
 */
static void positioning_configuration_handler_parse_config(const char *config)
{
  sl_status_t sc;
  aoa_id_t id;
  char *str_config;

  sc = aoa_parse_init(config);
  if (SL_STATUS_OK != sc) {
    sl_log_error(LOG_TAG, "Failed to parse config.");
    return;
  }

  sl_log_info(LOG_TAG, "Parsing positioning configuration:");

  sc = aoa_parse_string_config(&str_config, "id", NULL);
  if(SL_STATUS_OK == sc) {
    aoa_id_copy(id, str_config);
    sl_log_info(LOG_TAG, "Positioning Id is set to: %s", id);
    set_positioning_id(id);
  }

  sc = aoa_parse_string_config(&str_config, "estimationModeLocation", NULL);
  if (sc == SL_STATUS_OK) {
    sc = aoa_parse_estimation_mode_from_string(str_config, &aoa_loc_config.estimation_mode);
    if (sc == SL_STATUS_OK) {
      sl_log_info(LOG_TAG, "Location estimation mode set to: %s", str_config);
    } else {
      sl_log_error(LOG_TAG, "Failed to set location estimation mode to %s", str_config);
    }
  }

  sc = aoa_parse_string_config(&str_config, "validationModeLocation", NULL);
  if (sc == SL_STATUS_OK) {
    sc = aoa_parse_validation_mode_from_string(str_config, &aoa_loc_config.validation_method);
    if (sc == SL_STATUS_OK) {
      sl_log_info(LOG_TAG, "Location validation mode set to: %s", str_config);
    } else {
      sl_log_error(LOG_TAG, "Failed to set location validation mode to %s", str_config);
    }
  }

  sc = aoa_parse_float_config(&aoa_loc_config.estimation_interval_sec,
                              "estimationIntervalSec",
                              NULL);
  if (sc == SL_STATUS_OK) {
    sl_log_info(LOG_TAG, "Location estimation interval set to: %f",
                 aoa_loc_config.estimation_interval_sec);
  }

  sc = aoa_parse_bool_config(&aoa_loc_config.filtering_enabled,
                             "locationFiltering",
                             NULL);
  if (sc == SL_STATUS_OK) {
    sl_log_info(LOG_TAG, "Location filtering filtering set to: %s",
                 aoa_loc_config.filtering_enabled ? "enabled" : "disabled");
  }

  sc = aoa_parse_float_config(&aoa_loc_config.filtering_amount,
                              "locationFilteringWeight",
                              NULL);
  if (sc == SL_STATUS_OK) {
    sl_log_info(LOG_TAG, "Location filtering weight set to: %f",
                 aoa_loc_config.filtering_amount);
  }

  sc = aoa_parse_uint32_config(&angle_queue_config.sequence_ids,
                               "numberOfSequenceIds",
                               NULL);
  if (sc == SL_STATUS_OK) {
    sl_log_info(LOG_TAG, "Sequence id slots set to: %d",
                 angle_queue_config.sequence_ids);
  }

  sc = aoa_parse_uint32_config(&angle_queue_config.max_sequence_diff,
                               "maximumSequenceIdDiffs",
                               NULL);
  if (sc == SL_STATUS_OK) {
    sl_log_info(LOG_TAG, "Maximum sequence id difference set to: %d",
                 angle_queue_config.max_sequence_diff);
  }

  aoa_loc_config.max_sequence_diff = angle_queue_config.max_sequence_diff;

  sc = aoa_parse_deinit();
  if (SL_STATUS_OK != sc) {
    sl_log_error(LOG_TAG, "Failed to deinit config parser.");
  }
}

void positioning_configuration_handler_parse_config_file(const char *file_name)
{
  char *buffer;
  buffer = aoa_parse_load_file(file_name);
  if (buffer == NULL) {
    sl_log_error(LOG_TAG, "Failed to load file: %s", file_name);
    sl_log_warning(LOG_TAG, "Continue with default settings");
  } else {
    positioning_configuration_handler_parse_config(buffer);
    free(buffer);
  }
}
