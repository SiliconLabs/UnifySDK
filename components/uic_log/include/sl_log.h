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

/**
 * @defgroup sl_log Unify Logging system
 * @ingroup unify_components
 * @brief Logging library for Unify applications and components.
 *
 * Logging library for Unify based on boost::log and wrapped with C headers. The
 * logging system features log scoping and filtering. All Unify components
 * should use the logging system for printing messages.
 *
 * @{
 */

#ifndef LOG_H
#define LOG_H
#include <stdio.h>
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Log levels
 */
typedef enum sl_log_level {
  SL_LOG_DEBUG,
  SL_LOG_INFO,
  SL_LOG_WARNING,
  SL_LOG_ERROR,
  SL_LOG_CRITICAL
} sl_log_level_t;

// TODO: move array to string(hex) to utility function instead
/** log a byte array as hex string. */
#define sl_log_byte_arr(tag, lvl, _arr, _arr_len)                             \
  {                                                                           \
    char _tmp_log_arr[_arr_len * 2 + 1] = {0};                                \
    char *_cur_log_arr_idx              = _tmp_log_arr;                       \
    for (unsigned int _log_arr_count = 0; _log_arr_count < _arr_len;          \
         _log_arr_count++) {                                                  \
      _cur_log_arr_idx                                                        \
        += snprintf(_cur_log_arr_idx,                                         \
                    sizeof(_tmp_log_arr) - (_cur_log_arr_idx - _tmp_log_arr), \
                    "%02X",                                                   \
                    (_arr)[_log_arr_count]);                                  \
    }                                                                         \
    sl_log(tag, lvl, "%s\n", _tmp_log_arr);                                   \
  }

/**
 * @brief Set log level.
 *
 * @param level log level
 */
void sl_log_set_level(sl_log_level_t level);

/**
 * @brief Get log level.
 *
 * @return log level
 */
sl_log_level_t sl_log_get_level();

/**
 * @brief Set log level for a given tag.
 *
 * This level will override the log level set in \ref sl_log_set_level
 * To remove a tag specific log level use \ref sl_log_unset_tag_level
 *
 * @param tag tag to set log level for
 * @param level log level to set for the tag
 */
void sl_log_set_tag_level(const char *tag, sl_log_level_t level);

/**
 * @brief Remove tag specific log level for tag.
 *
 * By removing the tag specific log level,
 * the log level for the tag will use the log level set by \ref sl_log_set_level
 *
 * @param tag tag to unset specific log level for
 */
void sl_log_unset_tag_level(const char *tag);

/**
 * @brief Convert sl_log_level as string to sl_log_level_t.
 *
 * @param level string representation of sl_log_level_t, supported values are:
 *              "d", "debug"
 *              "i", "info"
 *              "w", "warning"
 *              "e", "error"
 *              "c", "critical"
 * @param result
 * @return sl_status_t
 */
sl_status_t sl_log_level_from_string(const char *level, sl_log_level_t *result);

/**
 * @brief Read configuration from config library.
 */
void sl_log_read_config();

/**
 * @brief Write to the log
 *
 * @param tag Log tag to use
 * @param level Log level
 * @param fmtstr Formatted string (printf style)
 * @param ... arguments for the string
 */
void sl_log(const char *const tag,
            sl_log_level_t level,
            const char *fmtstr,
            ...);

#define COLOR_START "\033[32;1m"
#define COLOR_END   "\033[0m"
// Logging macros for calling sl_log with levels
#define sl_log_debug(tag, fmtstr, ...) \
  sl_log(tag, SL_LOG_DEBUG, fmtstr, ##__VA_ARGS__)
#define sl_log_info(tag, fmtstr, ...) \
  sl_log(tag, SL_LOG_INFO, fmtstr, ##__VA_ARGS__)
#define sl_log_warning(tag, fmtstr, ...) \
  sl_log(tag, SL_LOG_WARNING, fmtstr, ##__VA_ARGS__)
#define sl_log_error(tag, fmtstr, ...) \
  sl_log(tag, SL_LOG_ERROR, fmtstr, ##__VA_ARGS__)
#define sl_log_critical(tag, fmtstr, ...) \
  sl_log(tag, SL_LOG_CRITICAL, fmtstr, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif  //LOG_H
/** @} end log */
