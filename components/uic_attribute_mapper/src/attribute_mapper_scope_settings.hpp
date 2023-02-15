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

/**
 * @defgroup attribute_mapper_scope_settings Scope settings helpers
 * @brief Helpers to read and interpret what to do based on scope settings
 *
 * This component provides helpers functions helping to decide what to do
 * based on scope settings.
 *
 * @{
 */

#ifndef ATTRIBUTE_MAPPER_SCOPE_SETTINGS_HPP
#define ATTRIBUTE_MAPPER_SCOPE_SETTINGS_HPP

#include "attribute_mapper_ast.hpp"
#include "attribute_store.h"
#include <stdbool.h>

/**
 * @brief Available values for the create_attributes setting.
 */
// Create attributes only if explicitly asked
enum ATTRIBUTE_CREATION_SETTINGS {
  // Create attribute only if explicity requested (Existence value type)
  CREATION_SETTING_E_ONLY = 0,
  // Create attribute for Reported or Existence value types.
  CREATION_SETTING_RE_VALUE_TYPES = 1,
  // Create attribute for Desired, Reported or Existence value types.
  CREATION_SETTING_DRE_VALUE_TYPES = 2
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Looks up in the Scope settings if clear_desired is enabled
 *
 * @param scope_settings the settings passed to the scope
 *
 * @returns true if setting not configured or configured to a non-zero value
 * @returns false if setting is configured and set to 0.
 */
bool is_clear_desired_value_enabled(
  const ast::scope_settings_t &scope_settings);

/**
 * @brief Looks up in the Scope settings if chain_reaction is enabled
 *
 * @param scope_settings the settings passed to the scope
 *
 * @returns true if setting not configured or configured to a non-zero value
 * @returns false if setting is configured and set to 0.
 */
bool is_chain_reaction_enabled(const ast::scope_settings_t &scope_settings);

/**
 * @brief Looks up in the Scope settings if create_attributes is enabled
 *
 * @param updated_value_type    Updated value type ('r', 'd' or 'e')
 * @param scope_settings        The settings passed to the scope
 * @param evaluation_result     Evaluation result of the assignment
 *
 * @returns true if setting not configured or configured to a non-zero value
 * @returns false if setting is configured and set to 0.
 */
bool should_create_attributes(char updated_value_type,
                              const ast::scope_settings_t &scope_settings,
                              result_type_t evaluation_result);

/**
 * @brief Looks up in the Scope settings if common_parent_type is configured
 *
 * @param engine_configured_type  Configured Common Parent Type for the Attribute
 *                                Mapper engine
 * @param scope_settings          The settings passed to the scope
 *
 * @returns attribute_store_type_t to use
 */
attribute_store_type_t get_common_parent_type_scope_configuration(
  attribute_store_type_t engine_configured_type,
  const ast::scope_settings_t &scope_settings);

#ifdef __cplusplus
}
#endif

#endif  //ATTRIBUTE_MAPPER_SCOPE_SETTINGS_HPP
/** @} end attribute_mapper_scope_settings */
