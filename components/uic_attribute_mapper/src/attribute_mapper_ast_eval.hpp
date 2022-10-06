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
#include <float.h>
#include <boost/optional.hpp>
#include "attribute.hpp"
#include "attribute_mapper_ast.hpp"
#include "attribute_mapper_ast_path_eval.hpp"

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

template<typename T> struct eval {
  public:
  using result_type = boost::optional<T>;

  /**
   * @brief Construct a new eval object
   *
   * @param context Context in which this evaluator should work.
   */
  explicit eval(const attribute_store::attribute context = 0) : context(context)
  {}

  /**
   * @brief the undefined keyword
   */
  result_type operator()(const nil &) const
  {
    return result_type();
  }

  /**
   * @brief a literal number
   */
  result_type operator()(const uint32_t n) const
  {
    return n;
  }

  /**
   * @brief a literal number
   */
  result_type operator()(const float n) const
  {
    return n;
  }

  /**
   * @brief An operation
   *
   * An operation which should be applied to the current result,
   * can be any operation +,-,*,/,&,| etc.
   *
   * @param x The operation
   * @param lhs Left hand side of the operation
   */
  result_type operator()(const operation &x, result_type lhs) const
  {
    if (x.operator_ == operator_or) {
      if (!context.is_valid()) {
        return result_type();
      }

      if (lhs) {
        return lhs;
      } else {
        return boost::apply_visitor(*this, x.operand_);
      }
    }

    result_type rhs = boost::apply_visitor(*this, x.operand_);
    if (rhs && lhs) {
      switch (x.operator_) {
        case operator_plus:
          return lhs.value() + rhs.value();
        case operator_minus:
          return lhs.value() - rhs.value();
        case operator_mult:
          return lhs.value() * rhs.value();
        case operator_div:
          return lhs.value() / rhs.value();
        case operator_equals:
          return trunc(1000. * lhs.value()) == trunc(1000. * rhs.value());
        case operator_left_shift:
          return lhs.value() < rhs.value();
        case operator_right_shift:
          return lhs.value() > rhs.value();
        case operator_bitand:
          return ((int)round(lhs.value())) & ((int)round(rhs.value()));
        case operator_bitor:
          return ((int)round(lhs.value())) | ((int)round(rhs.value()));
        case operator_bitxor:
          return ((int)round(lhs.value())) ^ ((int)round(rhs.value()));
        case operator_modulo:
          return fmod(lhs.value(), rhs.value());
        case operator_exponent:
          return pow(lhs.value(), rhs.value());
        case operator_neq:
          return lhs.value() != rhs.value();
        case operator_or:  //handled above
          break;
        default:
          break;
      }
    }
    return result_type();
  }
  /**
   * @brief Unary operation
   */
  result_type operator()(const signed_ &x) const
  {
    result_type rhs = boost::apply_visitor(*this, x.operand_);
    if (rhs) {
      switch (x.sign) {
        case '-':
          return -rhs.value();
        default:  // Assumed to be only '+' for now.
          return +rhs.value();
      }
    }
    return result_type();
  }

  /**
   * @brief Condition
   *
   * for if statements
   */
  result_type operator()(const expression &x) const
  {
    result_type state = boost::apply_visitor(*this, x.first);
    for (operation const &oper: x.rest) {
      state = (*this)(oper, state);
    }
    return state;
  }

  /**
   * @brief The normal entry point of the evaluator
   */
  result_type operator()(const condition &x) const
  {
    result_type cond = boost::apply_visitor(*this, x.cond_value);
    if (cond && cond.value()) {
      return boost::apply_visitor(*this, x.cond_true);
    } else {
      return boost::apply_visitor(*this, x.cond_false);
    }
  }

  /**
     * @brief an attribute read from the attribute store
     */
  result_type operator()(const attribute &a) const
  {
    if (!context.is_valid()) {
      return result_type();
    }

    attribute_path_eval ape(context);
    attribute_store::attribute node = ape(a.attribute_path);

    if (a.value_type == 'e') {  //existence evaluation
      //If all elements have bee parsed this means that we have found the attribute
      return ape.all_elements_parsed();
    } else if (ape.all_elements_parsed()) {
      T result = 0;
      if (a.value_type == 'd') {
        result = attribute_store_get_desired_number(node);
      } else if (a.value_type == 'r') {
        result = attribute_store_get_reported_number(node);
      }
      if (result != FLT_MIN) {
        return result;
      }
    }
    return result_type();
  }
  private:
  attribute_store::attribute context;
};

}  // namespace ast

#endif  //ATTRIBUTE_MAPPER_AST_EVAL_HPP

/**
 * @}
 */
