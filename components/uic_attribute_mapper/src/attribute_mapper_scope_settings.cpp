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
#include "attribute_mapper_scope_settings.hpp"

using namespace ast;

bool is_clear_desired_value_enabled(const scope_settings_t &scope_settings)
{
  for (const auto &setting: scope_settings) {
    if ((setting.setting_name == "clear_desired") && (setting.setting_value)) {
      return (setting.setting_value.value() != 0);
    }
  }

  return true;
}

bool is_chain_reaction_enabled(const scope_settings_t &scope_settings)
{
  for (const auto &setting: scope_settings) {
    if ((setting.setting_name == "chain_reaction") && (setting.setting_value)) {
      return (setting.setting_value.value() != 0);
    }
  }

  return true;
}

bool should_create_attributes(char updated_value_type,
                              const scope_settings_t &scope_settings,
                              result_type_t evaluation_result)
{
  if ((updated_value_type == 'e') && (evaluation_result != 0)) {
    return true;
  }

  int create_attribute_settings = CREATION_SETTING_RE_VALUE_TYPES;
  for (const auto &setting: scope_settings) {
    if ((setting.setting_name == "create_attributes")
        && (setting.setting_value)) {
      create_attribute_settings = setting.setting_value.value();
      break;
    }
  }

  if (create_attribute_settings == CREATION_SETTING_E_ONLY) {
    return false;
  }

  if (updated_value_type == 'r'
      && ((create_attribute_settings == CREATION_SETTING_DRE_VALUE_TYPES)
          || (create_attribute_settings == CREATION_SETTING_RE_VALUE_TYPES))) {
    return true;
  }

  if ((updated_value_type == 'd')
      && (create_attribute_settings == CREATION_SETTING_DRE_VALUE_TYPES)) {
    return true;
  }

  // In doubt, we do not create
  return false;
}

attribute_store_type_t get_common_parent_type_scope_configuration(
  attribute_store_type_t engine_configured_type,
  const scope_settings_t &scope_settings)
{
  for (const auto &setting: scope_settings) {
    if ((setting.setting_name == "common_parent_type")
        && (setting.setting_value)) {
      return setting.setting_value.value();
    }
  }

  return engine_configured_type;
}