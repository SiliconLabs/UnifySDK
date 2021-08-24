/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @file config.h
 * @defgroup config Config
 * @ingroup uic_components
 * @brief UIC Configuration System
 *
 * Configuration system for UIC Components/Applications.
 * It supports reading configuration from a YAML config file,
 * as well as from command line (arguments).
 * Command line arguments overrides the configurations from the config file.
 * 
 * Config variables are registed to the config system though the config_add_xxx
 * functions. When all variables have been registed the parsing of command line 
 * arguments and the configuration file is done using the function \ref config_parse.
 * After this the value of each configuration variable may be retrived using the
 * config_get_as_xxxx functions.
 * 
 * @{
 */

#ifndef CONFIG_H
#define CONFIG_H
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CONFIG_KEY_DATASTORE_FILE "datastore.file"
#define CONFIG_KEY_MQTT_HOST      "mqtt.host"
#define CONFIG_KEY_MQTT_PORT      "mqtt.port"
#define CONFIG_KEY_MQTT_CAFILE    "mqtt.cafile"
#define CONFIG_KEY_MQTT_CERTFILE  "mqtt.certfile"
#define CONFIG_KEY_MQTT_KEYFILE   "mqtt.keyfile"
#define CONFIG_KEY_LOG_LEVEL      "log.level"
#define CONFIG_KEY_LOG_TAG_LEVEL  "log.tag_level"

typedef enum {
  CONFIG_STATUS_OK             = 0x0,
  CONFIG_STATUS_NOK            = 0x1,
  CONFIG_STATUS_ERROR          = 0x2,
  CONFIG_STATUS_INVALID_TYPE   = 0x4,
  CONFIG_STATUS_DOES_NOT_EXIST = 0x8,
} config_status_t;

/**
 * @brief Add string configuration
 *
 * @param name Name of the config
 * @param help Help message (to print to command line)
 * @param default_value Default value for the configuration
 * @return config_status_t CONFIG_STATUS_OK if success
 */
config_status_t config_add_string(const char *name,
                                  const char *help,
                                  const char *default_value);

/**
 * @brief Add an integer configuration.
 *
 * @param name Name of the config
 * @param help Help message (to print to command line)
 * @param default_value Default value for the configuration
 * @return config_status_t CONFIG_STATUS_OK if success
 */
config_status_t
  config_add_int(const char *name, const char *help, int default_value);

/**
 * @brief Add a double configuration.
 *
 * @param name Name of the config
 * @param help Help message (to print to command line)
 * @param default_value Default value for the configuration
 * @return config_status_t CONFIG_STATUS_OK if success
 */
config_status_t
  config_add_double(const char *name, const char *help, double default_value);

/**
 * @brief Add a bool configuration.
 *
 * @param name Name of the config
 * @param help Help message (to print to command line)
 * @param default_value Default value for the configuration
 * @return config_status_t CONFIG_STATUS_OK if success
 */
config_status_t
  config_add_bool(const char *name, const char *help, bool default_value);

/**
 * @brief Parse command line arguments to the config system.
 *
 * This must be done before calling any of the config_get functions.
 * In addition all config_add calls shall be done before calling this function.
 *
 * @param argc Argument count
 * @param argv Arguments as array of (c) strings
 * @param version Version string to print if "version" argument is given
 * @return config_status_t CONFIG_STATUS_OK if success
 */
config_status_t config_parse(int argc, char **argv, const char *version);

// Config Getters

/**
 * @brief Get the config as a C-string.
 *
 * @param name Name of configuration
 * @param result [out] value read from configuration.
 *                The string is allocated in the config internal data structure,
 *                that's the reason for it is a const **,
 *                in addition after calling @ref config_parse (again),
 *                the string is invalid.
 * @return config_status_t CONFIG_STATUS_OK if success
 */
config_status_t config_get_as_string(const char *name, const char **result);

/**
 * @brief Get the config as an integer.
 *
 * @param name Name of configuration
 * @param result [out] value read from configuration.
 * @return config_status_t CONFIG_STATUS_OK if success
 */
config_status_t config_get_as_int(const char *name, int *result);

/**
 * @brief Get the config as a double.
 *
 * @param name Name of configuration
 * @param result [out] value read from configuration.
 * @return config_status_t CONFIG_STATUS_OK if success
 */
config_status_t config_get_as_double(const char *name, double *result);

/**
 * @brief Get the config as bool.
 *
 * @param name Name of configuration
 * @param result [out] value read from configuration.
 * @return config_status_t CONFIG_STATUS_OK if success
 */
config_status_t config_get_as_bool(const char *name, bool *result);

/**
 * @brief Reset the config.
 *
 * This will reset the config, so all added configurations are removed,
 * as well as all parsed configuration variables.
 * This is used in unit-tests, and there shouldn't be a reason to use it elsewhere
 */
void config_reset();

#ifdef __cplusplus
}
#endif

#endif  //CONFIG_H
/** @} end config */
