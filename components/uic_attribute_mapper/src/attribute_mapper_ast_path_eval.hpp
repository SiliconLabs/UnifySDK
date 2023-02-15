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
 * @defgroup attribute_mapper_ast_eval Abstract Syntax Tree Evaluation
 * @ingroup unify_attribute_mapper
 * @brief Evaluators that return the value of an AST
 *
 * @{
 */

#ifndef ATTRIBUTE_MAPPER_AST_PATH_EVAL_HPP
#define ATTRIBUTE_MAPPER_AST_PATH_EVAL_HPP

#include <string>
#include <map>
#include <float.h>
#include <boost/optional.hpp>
#include "attribute.hpp"
#include "attribute_mapper_ast.hpp"

namespace ast
{
struct nil;
struct signed_;
struct expression;
struct condition;
struct attribute;
struct attribute_path_subscript;
struct mapping;
struct assignment;

/**
   * @brief The attribute path evaluator evalues a full attribute path
   * and returns the matching attribute store attribute for that attribute
   * path.
   *
   * The evaluation is done in the context context.
   *
   */
struct attribute_path_eval {
  /**
   * @brief Construct a new attribute path eval object
   *
   * @param context parent on the attribute store
   */
  explicit attribute_path_eval(const attribute_store::attribute context) :
    context(context)
  {}

  /// Evaluates numbers and expressions into attributes
  attribute_store::attribute operator()(const ast::operand &oper);

  /// hat operator ^ (parent)
  attribute_store::attribute operator()(const nil &nul);

  /// just given by type
  attribute_store::attribute operator()(const attribute_store_type_t type_id);

  /// Subscript operator
  attribute_store::attribute
    operator()(const attribute_path_subscript &subscript) const;

  /// parse a path list return the last successfull evaluated attribute
  attribute_store::attribute operator()(
  const std::vector<attribute_path_element> &paths);

  /**
   * Return true if the path has been fully resolved
   */
  inline bool all_elements_parsed() const
  {
    return elements_left == 0;
  }

  /**
   *  return true if all but the last path elements has been resolved
   */
  inline bool last_token_failed() const
  {
    return elements_left == 1;
  }

  /**
   *  get the type is of the last element that was attempted to be parsed
   */
  inline attribute_store_type_t last_fail_type() const
  {
    return last_type_id;
  }

  /**
   *  Return the depth of the path, ie the number ofr path elemnts
   */
  size_t get_depth(const std::vector<attribute_path_element> &paths) const;

  private:
  //ID of last type passed
  attribute_store_type_t last_type_id = 0;
  int elements_left                   = 0;
  attribute_store::attribute context  = 0;
};

}  // namespace ast

#endif  //ATTRIBUTE_MAPPER_AST_EVAL_HPP

/**
 * @}
 */
