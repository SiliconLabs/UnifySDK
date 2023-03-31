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
 * @defgroup attribute_mapper_ast_dep_eval Abstract Syntax Tree Dependency Evaluation
 * @ingroup unify_attribute_mapper
 * @brief Dependency evaluators used by the mapping engine.
 *
 * @{
 */

#ifndef ATTRIBUTE_MAPPER_AST_DEP_EVAL_HPP
#define ATTRIBUTE_MAPPER_AST_DEP_EVAL_HPP

#include <set>
#include "attribute.hpp"
#include "attribute_mapper_ast_eval.hpp"

namespace ast
{
typedef char value_type_t;
typedef std::pair<attribute_store_type_t, value_type_t> attribute_dependency_t;
typedef std::vector<attribute_dependency_t> dependencies_t;

/**
 * @brief Dependency evaluator
 *
 * This evaluator goes through an AST starting with an expression and builds a
 * list of attribute id's that are used if the expression is to be evaluated.
 * Note that this evaluator accumulates a list of dependencies. Ie. each
 * instance of this class should only be used once. Otherwise the result will be
 * a combined set of dependencies for each expression this evaluator has been
 * applied on.
 */
class dep_eval
{
  public:
  const dependencies_t &operator()(nil)
  {
    return dependencies;
  }
  const dependencies_t &operator()(uint32_t)
  {
    return dependencies;
  }

  const dependencies_t &operator()(float)
  {
    return dependencies;
  }

  const dependencies_t &operator()(const attribute &a);
  const dependencies_t &operator()(const operation &x);
  const dependencies_t &operator()(const signed_ &x);
  const dependencies_t &operator()(const condition &x);
  const dependencies_t &operator()(const function_invokation &f);

  /**
   * @brief This is the normal entry point of the evaluator.
   *
   * @param x Top level expression to start from
   */
  const dependencies_t &operator()(const expression &x);

  /**
   * @brief Get the list of dependencies
   *
   * @return const std::vector<attribute_dependency_t>&
   */
  const dependencies_t &get_dependencies() const;

  private:
  dependencies_t dependencies;
};

/**
 * @brief Attribute path evaluator
 *
 * This evaluator build a list of attribute id's present in an
 * attribute path.
 */
class dep_eval_path
{
  public:
  /**
   * @brief Construct a new dep eval path object
   *
   * @param global_context Not currently used
   * @param value_type Desired or reported
   */
  dep_eval_path(value_type_t value_type);

  /// operand, usually a literal constant number
  const dependencies_t &operator()(const operand &operand);
  /// hat operator ^ (parent)
  const dependencies_t &operator()(const nil &nul)
  {
    return dependencies;
  }
  /// just given by type
  const dependencies_t &operator()(attribute_store_type_t type);
  /// Subscript operator
  const dependencies_t &operator()(const attribute_path_subscript &subscript);
  /// One path element
  const dependencies_t &operator()(const attribute_path_element &path_element);
  /// parse a path list,
  const dependencies_t &
    operator()(const std::vector<attribute_path_element> &paths);

  /**
   * @brief Get the dependencies list
   *
   * @return const std::vector<attribute_dependency_t>&
   */
  const dependencies_t &get_dependencies() const;

  private:
  dependencies_t dependencies;
  value_type_t value_type;
};

}  // namespace ast

#endif  //ATTRIBUTE_MAPPER_AST_DEP_EVAL_HPP

/**
 * @}
 */
