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

// Includes from this component
#include "attribute_mapper_ast.hpp"
#include "attribute_mapper_ast_eval.hpp"
#include "attribute_mapper_ast_path_eval.hpp"

// Unify components
#include "sl_log.h"

// Generic includes
#include <cmath>
#include <iostream>
namespace ast
{

/// hat operator ^ (parent)
attribute_store::attribute attribute_path_eval::operator()(const nil &)
{
  return context.parent();
}

/// just given by type
attribute_store::attribute
  attribute_path_eval::operator()(const attribute_store_type_t type_id)
{
  last_type_id                  = type_id;
  attribute_store_node_t new_id = context.child_by_type(type_id);

  return attribute_store::attribute(new_id);
}

//Evaluates numbers and expressions
attribute_store::attribute
  attribute_path_eval::operator()(const ast::operand &oper)
{
  ast::eval<uint32_t> evaluator(context);
  auto value = boost::apply_visitor(evaluator, oper);
  if (value) {
    attribute_store_node_t attribute_id = (*this)(value.value());
    return attribute_store::attribute(attribute_id);
  } else {
    return attribute_store::attribute();
  }
}

// Subscript operator
attribute_store::attribute attribute_path_eval::operator()(
  const attribute_path_subscript &subscript) const
{
  eval<uint32_t> evaluator(context);
  auto index   = boost::apply_visitor(evaluator, subscript.index);
  auto type_id = boost::apply_visitor(evaluator, subscript.identifier);
  if (index && type_id) {
    for (attribute_store::attribute child:
         context.children(static_cast<uint32_t>(type_id.value()))) {
      if (child.reported_exists() == false) {
        // Do not try to read the value if the reported value is undefined, as the
        // attribute_store_get_reported_number would return FLT_MIN,
        // and this would cast as 0 in uint32_t.
        continue;
      }
      result_type_t value = attribute_store_get_reported_number(child);
      if (static_cast<uint32_t>(value)
          == static_cast<uint32_t>(index.value())) {
        return child;
      }
    }
  }
  return attribute_store::attribute();
}

// parse a path list return the last successfully evaluated attribute
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
