/******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#ifndef USER_CREDENTIAL_CHECKSUM_CALCULATOR_H
#define USER_CREDENTIAL_CHECKSUM_CALCULATOR_H

// Cpp includes
#include <vector>

// Unify
#include "attribute.hpp"

// Types
#include "zwave_command_class_user_credential_types.h"

namespace user_credential
{

/**
 * @brief Compute the checksum based on the attribute store
 * 
 */
class checksum_calculator
{
  public:
  /**
  * @brief Compute a node value and add it to the current checksum
  * 
  * @param node Node to compute the checksum from
  * 
  * @throws std::runtime_error If the node is invalid or if the conversion failed
  */
  void add_node(const attribute_store::attribute &attribute);
  /**
   * @brief Compute the checksum of the added nodes
   * 
   * @note If no nodes are added, the checksum is 0 (see CC:0083.01.19.11.016 & CC:0083.01.17.11.013)
   * 
   * @return user_credential_checksum_t
   */
  user_credential_checksum_t compute_checksum();

  private:
  std::vector<uint8_t> checksum_data;
};

}  // namespace user_credential

#endif  // USER_CREDENTIAL_CHECKSUM_CALCULATOR_H