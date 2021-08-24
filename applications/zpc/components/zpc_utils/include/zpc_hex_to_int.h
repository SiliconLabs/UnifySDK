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
 * @file zpc_hex_to_int.h
 * @defgroup zpc_utils ZPC Utilities
 * @ingroup components
 * @brief hex to int convert functions and macros for ZPC
 *
  * @{
 */
#ifndef ZPC_HEX_TO_INT_H
#define ZPC_HEX_TO_INT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Convert a hex character to its int value 
 *
 * @param c: hex character
 * @return integer value of the character. On error return 0 
 */

int zpc_hex2int(char c);
#endif //ZPC_HEX_TO_INT_Hf
/** @} end zpc_hex_to_int*/

