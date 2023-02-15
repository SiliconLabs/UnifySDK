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
#include "mqtt_client_id.h"

// Unify components
#include "sl_log.h"
#include "config.h"
#include "datastore.h"

// Generic includes.
#include <string>
#include <string.h>
#include <algorithm>
#include <stdio.h>
#include <libgen.h>

// Constants
constexpr const char LOG_TAG[]   = "mqtt_client_id";
constexpr int MAX_CLIENT_ID_SIZE = 100;

// Static variables
namespace
{
// C-String array to save the datastore Client ID value.
char client_id[MAX_CLIENT_ID_SIZE] = {};
}  // namespace

////////////////////////////////////////////////////////////////////////////////
// Private helper functions
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Checks if we keep a character in the Client ID or not.
 *
 * @param character     ASCII value to check. Note that extended characters
 *                      will not play well with this function.
 *
 * @returns true if we will remove the character, false if we keep it.
 */
static bool filter_character_from_client_id(char character)
{
  if (character >= '0' && character <= '9') {
    return false;
  } else if (character >= 'A' && character <= 'Z') {
    return false;
  } else if (character >= 'a' && character <= 'z') {
    return false;
  } else if ((character == '_') || (character == '-')) {
    return false;
  }

  return true;
}

/**
 * @brief Cleans up a string of non-desired characters for the MQTT Client ID.
 *
 * @param [out] string_to_clean_up    Reference to the std::string to clean up
 */
static void
  remove_filtered_characters_from_string(std::string &string_to_clean_up)
{
  string_to_clean_up.erase(std::remove_if(string_to_clean_up.begin(),
                                          string_to_clean_up.end(),
                                          filter_character_from_client_id),
                           string_to_clean_up.end());
}

const char *mqtt_client_id_get_from_config()
{
  // Check if configuration indicates additional data to the Client ID.
  const char *config_client_id = nullptr;
  if (CONFIG_STATUS_OK
        != config_get_as_string(CONFIG_KEY_MQTT_CLIENT_ID, &config_client_id)
      || (config_client_id == nullptr)) {
    sl_log_critical(LOG_TAG,
                    "Error retrieving the MQTT client ID extension. "
                    "Please configure a valid MQTT Client ID using Unify "
                    "Configuration");
    return client_id;
  }

  // Sanitize the string:
  std::string client_id_string = std::string(config_client_id);
  remove_filtered_characters_from_string(client_id_string);

  // Clip to our maximum length:
  snprintf(client_id, sizeof(client_id), "%s", client_id_string.c_str());
  return client_id;
}

void mqtt_client_store_client_id_to_datastore(const char *client_id_in_use)
{
  if (false == datastore_is_initialized()) {
    sl_log_debug(LOG_TAG,
                 "No datastore active, MQTT Client ID will not "
                 "be saved in the datastore.");
    return;
  }

  // Check if there was a value before, and if it is different, scream!
  unsigned int datastore_string_length         = MAX_CLIENT_ID_SIZE;
  char datastore_client_id[MAX_CLIENT_ID_SIZE] = {};
  if (SL_STATUS_OK
      == datastore_fetch_arr(DATASTORE_MQTT_CLIENT_ID_KEY,
                             (uint8_t *)datastore_client_id,
                             &datastore_string_length)) {
    if (strcmp(datastore_client_id, client_id_in_use) != 0) {
      sl_log_warning(LOG_TAG,
                     "MQTT Client ID saved in the datastore (%s) does not "
                     "match new MQTT Client ID passed via the Configuration "
                     "(%s). Configuration value will be used.",
                     datastore_client_id,
                     client_id_in_use);
    }
  }

  // Store the value we use.
  sl_log_debug(LOG_TAG,
               "Storing MQTT Client ID (%s) used to connect to the broker "
               "in the datastore.",
               client_id_in_use);
  datastore_store_arr(DATASTORE_MQTT_CLIENT_ID_KEY,
                      (const uint8_t *)client_id_in_use,
                      strlen(client_id_in_use) + 1);
}
