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

namespace ast
{
dep_eval::dep_eval(const ast_tree &global_context) :
  global_context(global_context)
{}

void dep_eval::operator()(const attribute &a)
{
  dep_eval_path evaluator(global_context, a.value_type);
  evaluator(a.attribute);
  const std::set<attribute_dependency_t> &path_deps
    = evaluator.get_dependencies();

  dependencies.insert(path_deps.begin(), path_deps.end());
}

void dep_eval::operator()(const operation &x)
{
  boost::apply_visitor(*this, x.operand_);
}

void dep_eval::operator()(const signed_ &x)
{
  boost::apply_visitor(*this, x.operand_);
}

void dep_eval::operator()(const expression &x)
{
  boost::apply_visitor(*this, x.first);
  for (operation const &oper: x.rest) {
    (*this)(oper);
  }
}

void dep_eval::operator()(const condition &x)
{
  boost::apply_visitor(*this, x.cond_value);
  boost::apply_visitor(*this, x.cond_true);
  boost::apply_visitor(*this, x.cond_false);
}

const std::set<attribute_dependency_t> &dep_eval::get_dependencies()
{
  return dependencies;
}

dep_eval_path::dep_eval_path(const ast_tree &global_context,
                             value_type_t value_type) :
  global_context(global_context), value_type(value_type)
{}

void dep_eval_path::operator()(const ast::operand &operand)
{
  ast::eval evaluator(attribute_store::attribute(), global_context);
  auto value = boost::apply_visitor(evaluator, operand);
  if (value) {
    (*this)(value.value());
  } else {
    throw std::domain_error("attribute path elements must be constant.");
  }
}

void dep_eval_path::operator()(unsigned int type_id)
{
  dependencies.insert({type_id, value_type});
}

// Subscript operator
void dep_eval_path::operator()(const attribute_path_subscript &subscript)
{
  dep_eval dep_evaluator(global_context);
  boost::apply_visitor(dep_evaluator, subscript.index);
  boost::apply_visitor(dep_evaluator, subscript.identifier);
  dep_eval_path::operator()(subscript.identifier);

  const std::set<attribute_dependency_t> &path_deps
    = dep_evaluator.get_dependencies();

  dependencies.insert(path_deps.begin(), path_deps.end());
}

// parse a path list
void dep_eval_path::operator()(const std::vector<attribute_path_element> &paths)
{
  for (auto p: paths) {
    boost::apply_visitor(*this, p);
  }
}

const std::set<attribute_dependency_t> &dep_eval_path::get_dependencies()
{
  return dependencies;
}

}  // namespace ast
