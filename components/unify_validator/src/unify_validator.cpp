/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Labor                   atories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include "unify_validator.h"

#include <stdbool.h>
#include <cstring>
#include <regex>

bool validator_validate_string_contains_letter_or_digit(const char *value) {
  return std::regex_search(value, std::regex("[a-zA-Z0-9]+"));
}

bool validator_validate_unid(const char *value) {
  if (0 == strlen(value)) {
    return false;
  }

  // check if there is at least one digit or letter
  if (false == std::regex_search(value, std::regex("[a-zA-Z0-9]+"))) {
    return false;
  }

  // check if there is a space, slash or backslash
  if (true == std::regex_search(value, std::regex(R"([ ]|[\\]|[/])"))) {
    return false;
  }

  return true;
}
