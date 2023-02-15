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
 * @defgroup zwave_controller_endian Z-Wave Controller endian functions
 * @ingroup zwave_controller_utils
 * @brief These macros have the same functionality as the well know ntohl and ntohs
 * function from libc.
 *
 * @{
 */

#ifndef ZWAVE_CONTROLLER_ENDIAN_H
#define ZWAVE_CONTROLLER_ENDIAN_H

#ifdef __APPLE__
#include <machine/endian.h>
#define zwave_controller_ntohll(x) ntohll(x)
#define zwave_controller_ntohl(x)  ntohl(x)
#define zwave_controller_ntohs(x)  ntohs(x)
#else
#if !defined(ZWAVE_BUILD_SYSTEM)
#include <endian.h>
#else
#include <c_endianness.h>
#endif  // ZWAVE_BUILD_SYSTEM
#define zwave_controller_ntohll(x) be64toh(x)
#define zwave_controller_ntohl(x)  be32toh(x)
#define zwave_controller_ntohs(x)  be16toh(x)
#endif

#define zwave_controller_htohnll(x) zwave_controller_ntohll(x)
#define zwave_controller_htohnl(x)  zwave_controller_ntohl(x)
#define zwave_controller_htohns(x)  zwave_controller_ntohs(x)

#endif  //ZWAVE_CONTROLLER_ENDIAN_H
/** @} end zwave_controller_endian */
