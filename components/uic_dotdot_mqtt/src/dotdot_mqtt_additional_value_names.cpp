/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "dotdot_mqtt_additional_value_names.hpp"

// Includes from other components
#include "dotdot_attribute_id_definitions.h"
#include "dotdot_cluster_id_definitions.h"
#include "sl_log.h"

// Generic includes
#include <map>
#include <tuple>

// Private variables
namespace
{
static constexpr char LOG_TAG[] = "dotdot_mqtt_additional_value_names";

const std::map<std::tuple<dotdot_cluster_id_t, dotdot_attribute_id_t, int64_t>,
               std::string>
  additional_value_name_map {
    // State
    {{DOTDOT_STATE_CLUSTER_ID,
      DOTDOT_STATE_MAXIMUM_COMMAND_DELAY_ATTRIBUTE_ID,
      0xFFFFFFFE},
     "infinite"},
    {{DOTDOT_STATE_CLUSTER_ID,
      DOTDOT_STATE_MAXIMUM_COMMAND_DELAY_ATTRIBUTE_ID,
      0xFFFFFFFF},
     "unknown"},
    // Barrier Operator
    {{DOTDOT_BARRIER_CONTROL_CLUSTER_ID,
      DOTDOT_BARRIER_CONTROL_BARRIER_POSITION_ATTRIBUTE_ID,
      0xFF},
     "PositionUnknown"},
    // Color Control
    {{DOTDOT_COLOR_CONTROL_CLUSTER_ID,
      DOTDOT_COLOR_CONTROL_COLOR_TEMPERATURE_MIREDS_ATTRIBUTE_ID,
      0},
     "Undefined"},
    {{DOTDOT_COLOR_CONTROL_CLUSTER_ID,
      DOTDOT_COLOR_CONTROL_START_UP_COLOR_TEMPERATURE_MIREDS_ATTRIBUTE_ID,
      0xFFFF},
     "SetColorTempToPreviousValue"},
    // Door Lock
    {{DOTDOT_DOOR_LOCK_CLUSTER_ID,
      DOTDOT_DOOR_LOCK_AUTO_RELOCK_TIME_ATTRIBUTE_ID,
      0},
     "Disabled"},
    // Level
    {{DOTDOT_LEVEL_CLUSTER_ID,
      DOTDOT_LEVEL_START_UP_CURRENT_LEVEL_ATTRIBUTE_ID,
      0},
     "MinimumDeviceValuePermitted"},
    {{DOTDOT_LEVEL_CLUSTER_ID,
      DOTDOT_LEVEL_START_UP_CURRENT_LEVEL_ATTRIBUTE_ID,
      0xFF},
     "SetToPreviousValue"},
    // Scenes
    {{DOTDOT_SCENES_CLUSTER_ID, DOTDOT_SCENES_CURRENT_GROUP_ATTRIBUTE_ID, 0},
     "NotAssociatedWithScene"}};
}  // namespace

bool uic_dotdot_has_attribute_value_a_name(dotdot_cluster_id_t cluster_id,
                                           dotdot_attribute_id_t attribute_id,
                                           int64_t value)
{
  auto key = std::make_tuple(cluster_id, attribute_id, value);

  auto it = additional_value_name_map.find(key);
  if (it != additional_value_name_map.end()) {
    return true;
  }

  return false;
}

std::string
  uic_dotdot_get_attribute_value_name(dotdot_cluster_id_t cluster_id,
                                      dotdot_attribute_id_t attribute_id,
                                      int64_t value)
{
  auto key = std::make_tuple(cluster_id, attribute_id, value);
  auto it  = additional_value_name_map.find(key);

  if (it != additional_value_name_map.end()) {
    return it->second;
  }

  sl_log_warning(LOG_TAG,
                 "A component requested a name for an attribute value "
                 "that has none. This should not happen, they should use "
                 "uic_dotdot_has_attribute_value_a_name() before invoking "
                 "this function.");
  return std::string("");
}