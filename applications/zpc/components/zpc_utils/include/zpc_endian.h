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
 * @file zpc_endian.h
 * @ingroup zpc_utils
 * @brief Utility macros for endianness conversions
 *
 * Theese macros has the same functionality as the well know ntohl and ntohs
 * function from libc.
 *
 * @{
 */

#ifndef ZPC_ENDIAN_H
#define ZPC_ENDIAN_H

#ifdef __APPLE__
#include <machine/endian.h>
#define zpc_ntohll(x) ntohll(x)
#define zpc_ntohl(x) ntohl(x)
#define zpc_ntohs(x) ntohs(x)
#else
#if !defined(ZWAVE_BUILD_SYSTEM)
  #include <endian.h>
#else
  #include <c_endianness.h>
#endif  // ZWAVE_BUILD_SYSTEM
#define zpc_ntohll(x) be64toh(x)
#define zpc_ntohl(x)  be32toh(x)
#define zpc_ntohs(x)  be16toh(x)
#endif

#define zpc_htohnll(x) zpc_ntohll(x)
#define zpc_htohnl(x) zpc_ntohl(x)
#define zpc_htohns(x) zpc_ntohs(x)

#endif  //ZPC_ENDIAN_H
/** @} end zpc_endian */
