
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <string>

#include "sl_log.h"
#include "zigpc_common_dsk.hpp"

static const char LOG_TAG[] = "zigpc_commond_dsk";
/**
 * @brief Seperator used in DSKs
 *
 */
static const char DSK_SEP = '-';

sl_status_t zigpc_common::deserialize_dsk_install_code(
                    const std::string &dsk, 
                    dsk_install_code_t &dsk_content)
{
  sl_status_t status = SL_STATUS_OK;
  std::istringstream dsk_iss(dsk);
  std::ostringstream eui64_oss;
  int dsk_element;

  /* EUI64 parsing (fixed length) */
  for (uint8_t i = 0U; (status == SL_STATUS_OK) && (i < sizeof(zigbee_eui64_t));
       i++) {
    dsk_iss >> std::noshowbase >> std::hex >> dsk_element;
    if (dsk_iss.fail() || (dsk_element > 0xFF)) {
      sl_log_debug(LOG_TAG, "Unable to parse EUI64 from DSK");
      status = SL_STATUS_FAIL;
    } else {
      eui64_oss << std::uppercase << std::setfill('0') << std::setw(2)
                << std::hex << dsk_element;
      dsk_content.eui64[i] = static_cast<uint8_t>(dsk_element & 0xFF);
      dsk_iss.ignore(1, DSK_SEP);
    }
  }
  if (status == SL_STATUS_OK) {
    dsk_content.install_code_length = 0U;
    /* Install code parsing (variable length) */
    for (uint8_t i = 0U;
         (status == SL_STATUS_OK) && (i < ZIGBEE_INSTALL_CODE_LENGTH);
         i++) {
      dsk_iss >> std::noshowbase >> std::hex >> dsk_element;
      if (dsk_iss.fail() || (dsk_element > 0xFF)) {
        sl_log_debug(LOG_TAG, "Unable to parse Install Code from DSK");
        status = SL_STATUS_FAIL;
      } else {
        dsk_content.install_code[i] = static_cast<uint8_t>(dsk_element & 0xFF);
        if (dsk_iss.eof()) {
          dsk_content.install_code_length = i + 1;
          break;
        }
        dsk_iss.ignore(1, DSK_SEP);
      }
    }
    if (dsk_content.install_code_length == 0U) {
      return SL_STATUS_FAIL;
    }
  }
  
  if (status == SL_STATUS_OK) {
    dsk_content.dsk = dsk;
  }

  return status;
}

/**
 * @brief Comparison operator for DSK content based on install codes.
 *
 * @param lhs
 * @param rhs
 * @return true if members in content is equal, false otherwise.
 */
bool zigpc_common::operator==(
        const dsk_install_code_t &lhs,
        const dsk_install_code_t &rhs)
{
  bool equal = true;

  if (lhs.install_code_length != rhs.install_code_length) {
    equal = false;
  }

  if ((equal == true) && (lhs.dsk.compare(rhs.dsk) != 0)) {
    equal = false;
  }

  if ((equal == true)
      && (memcmp(lhs.eui64, rhs.eui64, sizeof(zigbee_eui64_t)))) {
    equal = false;
  }

  if ((equal == true)
      && (memcmp(lhs.install_code,
                 rhs.install_code,
                 sizeof(zigbee_install_code_t)))) {
    equal = false;
  }

  return equal;
}
