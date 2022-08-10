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
 * @file
 * @brief Wrapper module between app_log an sl_log components with limited API.
 */

#ifndef APP_LOG_H
#define APP_LOG_H

#include "sl_log.h"

#define APP_LOG_LEVEL_CRITICAL             SL_LOG_CRITICAL
#define APP_LOG_LEVEL_ERROR                SL_LOG_ERROR
#define APP_LOG_LEVEL_WARNING              SL_LOG_WARNING
#define APP_LOG_LEVEL_INFO                 SL_LOG_INFO
#define APP_LOG_LEVEL_DEBUG                SL_LOG_DEBUG

#define APP_LOG_NL                         "\n"

#define app_log_level(level, ...)          sl_log("GSDK", level, __VA_ARGS__)

#define app_log_debug(...)           \
  app_log_level(APP_LOG_LEVEL_DEBUG, \
                __VA_ARGS__)

#define app_log_info(...)           \
  app_log_level(APP_LOG_LEVEL_INFO, \
                __VA_ARGS__)

#define app_log_warning(...)           \
  app_log_level(APP_LOG_LEVEL_WARNING, \
                __VA_ARGS__)

#define app_log_error(...)           \
  app_log_level(APP_LOG_LEVEL_ERROR, \
                __VA_ARGS__)

#define app_log_critical(...)           \
  app_log_level(APP_LOG_LEVEL_CRITICAL, \
                __VA_ARGS__)

#endif // APP_LOG_H
