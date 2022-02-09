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

#ifndef ATTRIBUTE_MAPPER_AST_EVAL_HPP
#define ATTRIBUTE_MAPPER_AST_EVAL_HPP

#include <string>
#include <map>
#include <boost/optional.hpp>
#include "attribute.hpp"
#include "attribute_mapper_ast.hpp"

namespace ast
{
typedef boost::optional<int> result_type;

struct nil;
struct signed_;
struct expression;
struct condition;
struct attribute;
struct attribute_path_subscript;
struct mapping;
struct assignment;

/**
 * @brief AST evaluator
 *
 * This evaluator walks though the AST. Usually the evaluator is use to evaluate
 * a toplevel expression, but this can also be applied to other AST nodes.
 *
 * The evaluation is done in a context such that all attribute paths will be
 * evaluated relative to the context (a common parent node in the attribute
 * store).
 *
 * The AST evaluator returns a std::optional, because evaluation is not always
 * possible if the the needed attribute are not found within the context.
 *
 * If the evaluation context is an invalid attribute all evaluations involving
 * an attribute will return no value.
 *
 * The AST evaluators are implemented as a functor (a function object). This
 * because this allows us to use the boost::apply_visitor, which is able to
 * evaluate the boost::variant objects using functors, through the operator()
 * overloading. The use of the boost::apply_visitor is
 * boost::apply_visitor( functor, variant ), this will call the appropiate
 * operator() overloading for the data type in the variant.
 * See https://www.boost.org/doc/libs/1_76_0/doc/html/boost/apply_visitor.html
 * for details.
 */
struct eval {
  public:
  /**
   * @brief Construct a new eval object
   *
   * @param context Context in which this evaluator should work.
   */
  eval(const attribute_store::attribute context = 0);

  /**
   * @brief the undefined keyword
   */
  result_type operator()(const nil &) const;
  /**
   * @brief a literal number
   */
  result_type operator()(const unsigned int n) const;
  /**
   * @brief an attribute read from the attribute store
   */
  result_type operator()(const attribute &a) const;

  /**
   * @brief An operation
   *
   * An operation which should be applied to the current result,
   * can be any operation +,-,*,/,&,| etc.
   *
   * @param x The operation
   * @param lhs Left hand side of the operation
   */
  result_type operator()(const operation &x, result_type lhs) const;
  /**
   * @brief Unary operation
   */
  result_type operator()(const signed_ &x) const;
  /**
   * @brief Condition
   *
   * for if statements
   */
  result_type operator()(const condition &x) const;

  /**
   * @brief The normal entry point of the evaluator
   */
  result_type operator()(const expression &x) const;

  private:
  attribute_store::attribute context;
};

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
  attribute_path_eval(const attribute_store::attribute context);

  /// hat operator ^ (parent)
  attribute_store::attribute operator()(const operand &oper);
  /// hat operator ^ (parent)
  attribute_store::attribute operator()(const nil &nul);
  /// just given by type id
  attribute_store::attribute operator()(const unsigned int type_id);
  /// Subscript operator
  attribute_store::attribute
    operator()(const attribute_path_subscript &subscript) const;

  /// parse a path list return the last successfull evaluated attribute
  attribute_store::attribute
    operator()(const std::vector<attribute_path_element> &paths);

  /**
   * Return true if the path has been fully resolved
   */
  bool all_elements_parsed() const;

  /**
   *  return true if all but the last path elements has been resolved
   */
  bool last_token_failed() const;

  /**
   *  get the type is of the last element that was attempted to be parsed
   */
  attribute_store_type_t last_fail_type() const;

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
