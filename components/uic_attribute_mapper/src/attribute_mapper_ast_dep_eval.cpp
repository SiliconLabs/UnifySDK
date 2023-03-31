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

#include "attribute_mapper_ast_dep_eval.hpp"
#include "attribute_store_helper.h"

namespace ast
{
const dependencies_t &dep_eval::operator()(const attribute &a)
{
  dep_eval_path evaluator(a.value_type);
  evaluator(a.attribute_path);
  const std::vector<attribute_dependency_t> &path_deps
    = evaluator.get_dependencies();

  dependencies.insert(dependencies.end(), path_deps.begin(), path_deps.end());
  return dependencies;
}

const dependencies_t &dep_eval::operator()(const operation &x)
{
  boost::apply_visitor(*this, x.operand_);
  return dependencies;
}

const dependencies_t &dep_eval::operator()(const signed_ &x)
{
  boost::apply_visitor(*this, x.operand_);
  return dependencies;
}

const dependencies_t &dep_eval::operator()(const expression &x)
{
  boost::apply_visitor(*this, x.first);
  for (operation const &oper: x.rest) {
    (*this)(oper);
  }
  return dependencies;
}

const dependencies_t &dep_eval::operator()(const condition &x)
{
  boost::apply_visitor(*this, x.cond_value);
  boost::apply_visitor(*this, x.cond_true);
  boost::apply_visitor(*this, x.cond_false);
  return dependencies;
}

const dependencies_t &dep_eval::operator()(const function_invokation &f)
{
  for (auto &argument: f.arguments) {
    (*this)(argument);
  }
  return dependencies;
}

const dependencies_t &dep_eval::get_dependencies() const
{
  return dependencies;
}

dep_eval_path::dep_eval_path(value_type_t value_type) : value_type(value_type)
{}

const dependencies_t &dep_eval_path::operator()(const operand &operand)
{
  ast::eval<uint32_t> evaluator;
  auto value = boost::apply_visitor(evaluator, operand);

  if (value) {
    (*this)(value.value());
  } else {
    throw std::domain_error("attribute path elements must be constant.");
  }
  return dependencies;
}

const dependencies_t &dep_eval_path::operator()(attribute_store_type_t type)
{
  dependencies.push_back({type, value_type});
  return dependencies;
}

// Subscript operator
const dependencies_t &
  dep_eval_path::operator()(const attribute_path_subscript &subscript)
{
  dep_eval dep_evaluator;
  boost::apply_visitor(dep_evaluator, subscript.index);
  boost::apply_visitor(dep_evaluator, subscript.identifier);
  dep_eval_path::operator()(subscript.identifier);

  const dependencies_t &path_deps = dep_evaluator.get_dependencies();

  dependencies.insert(dependencies.end(), path_deps.begin(), path_deps.end());
  return dependencies;
}

const dependencies_t &
  dep_eval_path::operator()(const attribute_path_element &path_element)
{
  return boost::apply_visitor(*this, path_element);
}

// parse a path list
const dependencies_t &
  dep_eval_path::operator()(const std::vector<attribute_path_element> &paths)
{
  size_t paths_size = paths.size();
  for (const auto &p: paths) {
    // Countdown to know when we are at the end of the vector
    paths_size -= 1;
    // If we are not at the end of the path and it is a simple attribute
    // without subscript, do not add it to the dependencies:
    if ((paths_size > 0) && (p.which() == 1)) {
      // p.which() is the index of the type in the attribute_path_element
      // value 1 means that the variant contains uint32_t value, so an attribute type directly.
    } else {
      (*this)(p);
    }
  }
  return dependencies;
}

const dependencies_t &dep_eval_path::get_dependencies() const
{
  return dependencies;
}

}  // namespace ast
