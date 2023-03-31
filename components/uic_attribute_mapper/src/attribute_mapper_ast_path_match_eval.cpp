/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "attribute_mapper_ast_path_match_eval.hpp"
#include "attribute_mapper_ast_eval.hpp"
#include "attribute_store_helper.h"
#include "sl_log.h"

namespace ast
{
///////////////////////////////////////////////////////////////////////////////
// path_match_eval Class Implementation
//////////////////////////////////////////////////////////////////////////////
path_match_eval::path_match_eval(attribute_store_node_t _attribute_to_match,
                                 attribute_store_type_t _common_parent_type) :
  attribute_to_match(_attribute_to_match),
  common_parent_type(_common_parent_type),
  current_attribute(_attribute_to_match)
{}

void path_match_eval::reset()
{
  this->current_attribute = this->attribute_to_match;
  this->comparing_value   = false;
}

/// Operand
bool path_match_eval::operator()(const ast::operand &operand)
{
  ast::eval<uint32_t> evaluator;
  auto value = boost::apply_visitor(evaluator, operand);
  if (value) {
    if (this->comparing_value == true) {
      return (value.value()
              == static_cast<attribute_store_type_t>(
                attribute_store_get_reported_number(this->current_attribute)));
    } else {
      return (value.value()
              == attribute_store_get_node_type(this->current_attribute));
    }
  } else {
    sl_log_debug("path_match_eval",
                 "Cannot derive value from operand. Returning false.");
    return false;
  }
}

/// Nil elements.
bool path_match_eval::operator()(const nil)
{
  this->current_attribute
    = attribute_store_get_node_parent(this->current_attribute);
  return attribute_store_node_exists(this->current_attribute);
}

/// Attribute Store type
bool path_match_eval::operator()(attribute_store_type_t type)
{
  if (this->comparing_value == true) {
    return (type
            == static_cast<attribute_store_type_t>(
              attribute_store_get_reported_number(this->current_attribute)));
  } else {
    return (type == attribute_store_get_node_type(this->current_attribute));
  }
}

/// Subscript operator
bool path_match_eval::operator()(const attribute_path_subscript &subscript)
{
  this->comparing_value = true;
  bool value            = (*this)(subscript.index);
  this->comparing_value = false;
  if (value == false) {
    return false;
  }
  return (*this)(subscript.identifier);
}

/// One path element
bool path_match_eval::operator()(const attribute_path_element &path_element)
{
  return boost::apply_visitor(*this, path_element);
}

/// A vector of path elements
bool path_match_eval::operator()(
  const std::vector<attribute_path_element> &paths)
{
  if (paths.size() == 0) {
    sl_log_warning("path_match_eval",
                   "Empty path to evaluate path match. Returning false");
    return false;
  }

  // Start at the end of the vector, making our way up
  for (size_t index = 0; index < paths.size(); index++) {
    const size_t i = paths.size() - 1 - index;
    if (false == (*this)(paths[i])) {
      return false;
    } else {
      this->current_attribute
        = attribute_store_get_node_parent(this->current_attribute);
    }
  }

  // Done parsing the chain, we check the common parent type:
  if (attribute_store_get_node_type(this->current_attribute)
      != this->common_parent_type) {
    return false;
  }

  return true;
}

}  // namespace ast
