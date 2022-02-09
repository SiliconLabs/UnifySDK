/* © 2017 Silicon Laboratories Inc.
 */
/**
 * @file aes_cmac.h
 * @author Christian Salmony Olsen
 * @date 22/06/2015
 * @brief Serves functions for calculating and verifying an AES-CMAC.
 * @details Based on the source code by Brian Gladman found in
 * https://code.google.com/p/kirk-engine/source/browse/trunk/cmac.c?r=9.
 *
 * The following licensing is taken from Brian's cmac header file.
 *
 * ############################################################################
 * ---------------------------------------------------------------------------
 * Copyright (c) 1998-2008, Brian Gladman, Worcester, UK. All rights reserved.
 *
 * LICENSE TERMS
 *
 * The redistribution and use of this software (with or without changes)
 * is allowed without the payment of fees or royalties provided that:
 *
 *  1. source code distributions include the above copyright notice, this
 *     list of conditions and the following disclaimer;
 *
 *  2. binary distributions include the above copyright notice, this list
 *     of conditions and the following disclaimer in their documentation;
 *
 *  3. the name of the copyright holder is not used to endorse products
 *     built using this software without specific written permission.
 *
 * DISCLAIMER
 *
 * This software is provided 'as is' with no explicit or implied warranties
 * in respect of its properties, including, but not limited to, correctness
 * and/or fitness for purpose.
 * ---------------------------------------------------------------------------
 * Issue Date: 6/10/2008
 *
 * ############################################################################
 *
 */

#ifndef AES_CMAC_H_
#define AES_CMAC_H_


#include <stdint.h>

/**
 * \ingroup crypto
 * @{
 */


/**
 * Type defining the return values of a CMAC verification.
 */
typedef enum
{
  CMAC_VALID = 0, //!< VALID
  CMAC_INVALID = 1//!< INVALID
}
CMAC_VERIFY_T;

/**
 * @brief Calculates an AES-CMAC from a given key, message and message length.
 * @param key Pointer to a 128-bit key.
 * @param message Pointer to the message to be authenticated.
 * @param message_length Length of the message in octets.
 * @param mac Pointer to an array where the calculated AES-CMAC can be stored.
 */
void aes_cmac_calculate(
        const uint8_t * key,
        const uint8_t * message,
        const uint16_t message_length,
        uint8_t * mac);

/**
 * @brief Verifies a given AES-CMAC from a given key, message and message length.
 * @param key Pointer to a 128-bit key.
 * @param message Pointer to the message to be authenticated.
 * @param message_length Bit length of the message in octets.
 * @param mac Pointer to a MAC to verify against.
 * @return Returns CMAC_VALID if valid, or CMAC_INVALID if not.
 */
CMAC_VERIFY_T aes_cmac_verify(
        const uint8_t * key,
        const uint8_t * message,
        const uint16_t message_length,
        const uint8_t * mac);

/**
 * @}
 */

#endif // AES_CMAC_H_
