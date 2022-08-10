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
 * @defgroup attribute_mapper_ast_reducer Abstract Syntax Tree Reducer
 * @ingroup unify_attribute_mapper
 * @brief Write a description about this
 *
 * @{
 */

#ifndef ATTRIBUTE_MAPPER_AST_REDUCER_H
#define ATTRIBUTE_MAPPER_AST_REDUCER_H

#include <string>
#include <map>
#include <iostream>
#include <boost/optional.hpp>
#include "attribute.hpp"
#include "attribute_mapper_ast.hpp"
#include "attribute_mapper_ast_eval.hpp"

namespace ast
{
/**
 * @brief AST reducer
 *
 * This class is an ast evaluator which outputs a copy of the ast in reduced
 * form.
 *
 * The ast reducer works by going though the ast and try evaluate operands with
 * a normal value evaluator which has an undefined context. This means that the
 * evaluator will only succeed if the operand is a constant, ie does not depend
 * on an attribute.
 *
 * As the other evaluators this evaluator operates on a functor which uses
 * pattern matchin to evaluate the boost::variant ast components using the
 * boost::apply_visitor.
 */
class reducer
{
  public:
  reducer() : evaluator(ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE) {};

  operand operator()(const nil &x) const
  {
    return operand(x);  //Cannot be reduced
  }

  operand operator()(uint32_t n) const
  {
    return operand(n);  //Cannot be reduced
  }

  operand operator()(float n) const
  {
    return operand(n);  //Cannot be reduced
  }

  operand operator()(const attribute &a) const
  {
    return operand(a);  //Cannot be reduced
  }

  operand operator()(const signed_ &x) const
  {
    if (x.sign == '+') {  // Unary + can just be replaced by its operand
      return boost::apply_visitor(*this, x.operand_);
    } else {
      signed_ s;
      s.sign     = x.sign;
      s.operand_ = boost::apply_visitor(*this, x.operand_);
      return operand(s);
    }
  }

  operand operator()(const condition &x) const
  {
    //Check if we can statically reduce cond_value
    auto val = boost::apply_visitor(evaluator, x.cond_value);
    if (val) {
      return val.value() ? x.cond_true : x.cond_false;
    } else {
      condition cond;
      cond.cond_value = boost::apply_visitor(*this, x.cond_value);
      cond.cond_true  = boost::apply_visitor(*this, x.cond_true);
      cond.cond_false = boost::apply_visitor(*this, x.cond_false);
      return operand(cond);
    }
  }

  operand operator()(const expression &x) const
  {
    return operand(reduce_expression(x));
  }

  /**
   * @brief This function reduces an operation
   *
   * this is not one of the boost::variant operators since the operation data type
   * is not an operand
   *
   * @param x  input operation
   * @return reduced operation
   */
  operation reduce_operation(const operation &x) const
  {
    operation o = x;
    auto val    = boost::apply_visitor(evaluator, x.operand_);
    if (val) {
      o.operand_ = operand(val.value());
    } else {
      o.operand_ = boost::apply_visitor(*this, x.operand_);
    }
    return o;
  }

  /**
   * @brief Reduce an expression
   *
   * This is the main entrypoint of reducer. An expession is also a operand
   * though the pattern "( <expression> )". So this reducer is accessed
   * though the operator()(const expression &x).
   *
   * @param x input expression
   * @return reduced expression
   */
  expression reduce_expression(const expression &x) const
  {
    expression reduced_expr;
    operator_ids unity_operation = ast::operator_plus;
    //Accumulator to store collected constants
    result_type_t acc = 0;
    bool substitute   = false;

    reduced_expr.first = boost::apply_visitor(*this, x.first);

    if (x.rest.empty()) {
      return reduced_expr;
    }

    // We only try to substitute + - and * /
    switch (x.rest.begin()->operator_) {
      case ast::operator_plus:
      case ast::operator_minus:
        acc             = 0;
        substitute      = true;
        unity_operation = ast::operator_plus;
        break;
      case ast::operator_mult:
      case ast::operator_div:
        acc             = 1;
        substitute      = true;
        unity_operation = ast::operator_mult;
        break;
      default:
        acc = 0;
        //We dont are substituing the other operators
        substitute = false;
    }

    // If the first operand is a constant then we can use this for the inital
    // accumulator value
    auto lhs = boost::apply_visitor(evaluator, reduced_expr.first);
    if (lhs) {
      acc = lhs.value();
    }

    // Go though all terms, if they are constant merge them into the accumulator
    for (const auto &op: x.rest) {
      auto val = evaluator(op, acc);
      // Don't substitute the div operator due to risk of rounding errors
      if (substitute && val && (op.operator_ != ast::operator_div)) {
        acc = val.value();
      } else {
        reduced_expr.rest.push_back(reduce_operation(op));
      }
    }

    if (substitute) {
      if (lhs) {
        reduced_expr.first = acc;
      } else {
        if (acc < 0) {
          signed_ s;
          s.sign             = '-';
          s.operand_         = operand(-acc);
          operation const_op = {unity_operation, operand(s)};
          reduced_expr.rest.push_back(const_op);
        } else {
          operation const_op = {unity_operation, operand(acc)};
          reduced_expr.rest.push_back(const_op);
        }
      }
    }
    return reduced_expr;
  }

  //Reduce the assignment by reducing the right hand side
  assignment operator()(const assignment &x) const
  {
    assignment reduced = x;
    reduced.rhs        = reduce_expression(x.rhs);
    return reduced;
  }

  //Reduce the assignments in a scope
  scope operator()(const scope &x) const
  {
    scope reduced = x;
    for (auto &a: reduced.assignments) {
      a = (*this)(a);
    }
    return reduced;
  }

  //Go though the ast an reduce each element
  ast_tree operator()(const ast_tree ast) const
  {
    ast_tree reduced = ast;
    for (auto &ast_element: reduced) {
      ast_element = boost::apply_visitor(*this, ast_element);
    }
    return reduced;
  }

  private:
  eval<result_type_t> evaluator;
};
}  // namespace ast
#endif
/** @} end attribute_mapper_ast_reducer */
