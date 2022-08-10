/**
 * @file
 *         Contiki configuration for FreeRTOS
 * @copyright
 *         Copyright (c) 2021, Silicon Labs., All Rights Reserved
 */

#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#include <inttypes.h>
#include "projdefs.h"

#define CC_CONF_REGISTER_ARGS           1
#define CC_CONF_FUNCTION_POINTER_ARGS   1
#define CC_CONF_FASTCALL
#define CC_CONF_VA_ARGS                 1
#define CC_CONF_NO_VA_ARGS              (!CC_CONF_VA_ARGS) // Stating the obvious because cc.h checks this first
#define CC_CONF_ALIGN_PACK              1
#define CC_CONF_REENTRANT_ARGS          0
#define CC_CONF_UNSIGNED_CHAR_BUGS      0
#define CC_CONF_DOUBLE_HASH             0

#define CCIF
#define CLIF

typedef uint8_t         u8_t;
typedef uint16_t        u16_t;
typedef uint32_t        u32_t;
typedef int32_t         s32_t;
typedef uint32_t clock_time_t;

#define CLOCK_CONF_SECOND               1000

#define LOG_CONF_ENABLED                0

#define PROCESS_CONF_NO_PROCESS_NAMES   0

#ifdef PROJECT_CONF_H
#include PROJECT_CONF_H
#endif /* PROJECT_CONF_H */

#endif /* __CONTIKI_CONF_H__ */
