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
 * @defgroup uic_validator Unify Validator component
 * @ingroup unify_components
 * @brief Unify validator
 *
 * This component provides a single interface to validate different types of data that present in Unify system.
 * It should be used in different applications and components.
 * @{
 */

#ifndef UIC_VALIDATOR_H
#define UIC_VALIDATOR_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Validates that string contains at least one digit or letter (either upper or lower case).
 *
 * @param value string to be validated
 */
bool validator_validate_string_contains_letter_or_digit(const char *value);

/**
 * @brief Validates UNID
 * It should contain at least one digit or letter (either upper or lower case).
 * It can't contain neither symbols: space, slash, backslash.
 *
 * @param value string that contains UNID to be validated
 */
bool validator_validate_unid(const char *value);

#ifdef __cplusplus
}
#endif

#endif  // UIC_VALIDATOR_H
/** @} end validator */
