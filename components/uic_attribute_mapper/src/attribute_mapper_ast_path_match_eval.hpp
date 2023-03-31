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

/**
 * @defgroup Attribute Mapper path match evaluation
 * @brief Class assessing if expression can match attributes in the Attribute Store
 *
 * Let's say I have an attribute, and a common parent type.
 * I have to check if my attribute/common parent type could be matching the
 * attribute path of an expression.
 *
 * For example, a.b[0].c, if I have an attribute of type c, does it fit, so is
 * the parent of type b with value 0, and the grand-parent of type a, and
 * the grand-grand parent of type "common_parent_type"
 *
 * @{
 */

#ifndef ATTRIBUTE_MAPPER_AST_PATH_MATCH_EVAL_HPP
#define ATTRIBUTE_MAPPER_AST_PATH_MATCH_EVAL_HPP

#include "attribute.hpp"
#include "attribute_mapper_ast.hpp"

namespace ast
{

/**
 * @brief Path Match Evaluation Class
 *
 */
class path_match_eval
{
  public:
  /**
   * @brief Construct a new dep eval path object
   *
   * @param _attribute_to_match Attribute Store Node to match to a path
   * @param _common_parent_type Attribute Store Type configured for the commong parent
   *
   */
  path_match_eval(attribute_store_node_t _attribute_to_match,
                  attribute_store_type_t _common_parent_type);

  /**
   * @brief Resets the navigation data, so it can perform a new evaluation.
   */
  void reset();

  // AST traversal functions
  bool operator()(const ast::operand &operand);
  bool operator()(const nil);
  bool operator()(attribute_store_type_t type);
  bool operator()(const attribute_path_subscript &subscript);
  bool operator()(const attribute_path_element &path_element);
  bool operator()(const std::vector<attribute_path_element> &paths);

  private:
  // Attribute initially set as the final destination
  attribute_store_node_t attribute_to_match;
  // Common parent type configured in the scope.
  attribute_store_type_t common_parent_type;
  // Attribute used for navigation.
  attribute_store_node_t current_attribute;
  // Boolean indicating if we compare attribute value or type. True for value, false for type.
  bool comparing_value = false;
};

}  // namespace ast

#endif  //ATTRIBUTE_MAPPER_AST_PATH_MATCH_EVAL_HPP
/** @} end attribute_mapper_ast_path_match_eval */
