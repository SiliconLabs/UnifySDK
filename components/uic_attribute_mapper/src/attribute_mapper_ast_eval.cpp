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
 * @file attribute_mapper_ast_eval.hpp
 * @brief TODO: Write brief for attribute_mapper_ast_eval
 *
 * TODO: Write component description for attribute_mapper_ast_eval
 *
 * @{
 */

#include "attribute_mapper_ast.hpp"
#include "attribute_mapper_ast_eval.hpp"
#include "sl_log.h"

namespace ast
{
eval::eval(const attribute_store::attribute context) : context(context) {}

result_type eval::operator()(const nil &) const
{
  return result_type();
}

result_type eval::operator()(const unsigned int n) const
{
  return n;
}

result_type eval::operator()(const attribute &a) const
{
  attribute_path_eval ape(context);
  attribute_store::attribute value = ape(a.attribute_path);

  if (a.value_type == 'e') {  //existence evaluation
    return ape
      .all_elements_parsed();  //If all elements have bee parsed this means that we have found the attribute
  } else if (ape.all_elements_parsed()) {
    try {
      if (a.value_type == 'd') {
        return value.desired<int32_t>();
      } else if (a.value_type == 'r') {
        return value.reported<int32_t>();
      }
    } catch (std::exception &e) {
      // sl_log_error(LOG_TAG, e.what()); // The mapper engine is based on trial and error concept which means these error messages are spammed.
    }
  }
  return result_type();
}

result_type eval::operator()(const operation &x, result_type lhs) const
{
  if (x.operator_ == operator_or) {
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
      case operator_comp:
        return lhs.value() == rhs.value();
      case operator_left_shift:
        return lhs.value() < rhs.value();
      case operator_right_shift:
        return lhs.value() > rhs.value();
      case operator_bitand:
        return lhs.value() & rhs.value();
      case operator_bitor:
        return lhs.value() | rhs.value();
      case operator_bitxor:
        return lhs.value() ^ rhs.value();
      case operator_or:  //handled above
        break;
        ;
    }
  }
  return result_type();
}

result_type eval::operator()(const signed_ &x) const
{
  result_type rhs = boost::apply_visitor(*this, x.operand_);
  if (rhs) {
    switch (x.sign) {
      case '-':
        return -rhs.value();
      case '+':
        return +rhs.value();
    }
  }
  return result_type();
}

result_type eval::operator()(const expression &x) const
{
  result_type state = boost::apply_visitor(*this, x.first);
  for (operation const &oper: x.rest) {
    state = (*this)(oper, state);
  }
  return state;
}

result_type eval::operator()(const condition &x) const
{
  result_type cond = boost::apply_visitor(*this, x.cond_value);
  if (cond && cond.value()) {
    return boost::apply_visitor(*this, x.cond_true);
  } else {
    return boost::apply_visitor(*this, x.cond_false);
  }
}

attribute_path_eval::attribute_path_eval(
  const attribute_store::attribute context) :
  context(context)
{}

// hat operator ^ (parent)
attribute_store::attribute
  attribute_path_eval::operator()(const ast::operand &oper)
{
  ast::eval evaluator(context);
  auto value = boost::apply_visitor(evaluator, oper);
  if (value) {
    unsigned int attribute_id = (*this)(value.value());
    return attribute_id;
  } else {
    return attribute_store::attribute();
  }
}

// hat operator ^ (parent)
attribute_store::attribute attribute_path_eval::operator()(const nil &nul)
{
  return context.parent();
}

// just given by type id
attribute_store::attribute
  attribute_path_eval::operator()(const unsigned int type_id)
{
  last_type_id        = type_id;
  unsigned int new_id = context.child_by_type(type_id);

  return new_id;
}

// Subscript operator
attribute_store::attribute attribute_path_eval::operator()(
  const attribute_path_subscript &subscript) const
{
  eval evaluator(context);
  auto index = boost::apply_visitor(evaluator, subscript.index);
  if (index) {
    auto type_id
      = boost::apply_visitor(evaluator, subscript.identifier);  //TODO do lookup
    if (type_id) {
      attribute_store::attribute child
        = context.child_by_type_and_value(type_id.value(), index.value());
      return child;
    }
  }
  return attribute_store::attribute();
}

// parse a path list return the last successfull evaluated attribute
//
attribute_store::attribute attribute_path_eval::operator()(
  const std::vector<attribute_path_element> &paths)
{
  elements_left                = paths.size();
  attribute_store::attribute a = context;

  for (auto p: paths) {
    a = boost::apply_visitor(*this, p);
    if (!a.is_valid()) {
      break;
    } else {
      context = a;
      elements_left--;
    }
  }
  return context;
}

//Return true if the path has been fully resolved
bool attribute_path_eval::all_elements_parsed() const
{
  return elements_left == 0;
}

// return true if all but the last path elements has been resolved
bool attribute_path_eval::last_token_failed() const
{
  return elements_left == 1;
}
// get the type is of the last element that was attempted to be parsed
attribute_store_type_t attribute_path_eval::last_fail_type() const
{
  return last_type_id;
}

size_t attribute_path_eval::get_depth(
  const std::vector<attribute_path_element> &paths) const
{
  size_t depth = paths.size();
  for (auto p: paths) {
    if (p.type() == typeid(ast::nil))
      depth--;
  }
  return depth;
}

}  // namespace ast
