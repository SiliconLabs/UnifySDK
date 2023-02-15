/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef ATTRIBUTE_MAPPER_BUILT_IN_FUNCTIONS_HPP
#define ATTRIBUTE_MAPPER_BUILT_IN_FUNCTIONS_HPP

#include "attribute_mapper_ast.hpp"

#include <boost/optional.hpp>
#include <map>
#include <string>
#include <vector>
#include <functional>

#include "sl_log.h"

namespace ast
{
// Define the result type. Here we keep the boost::optional<T> from attribute_mapper_ast_eval.hpp
template<typename T> using eval_result_type = boost::optional<T>;
template<typename T> using built_in_function
  = std::function<eval_result_type<T>(std::vector<eval_result_type<T>> &)>;
template<typename T> using mapper_function_map
  = std::map<std::string, built_in_function<T>>;

/**
 * @brief Gets the map of built-in functions, containing names and pointers
 *
 * @return const mapper_function_map
 */
template<typename T> const mapper_function_map<T> &get_built_in_functions();

/**
 * @brief Class that evaluates if we have unknown functions in the AST.
 */
class built_in_function_check
{
  public:
  bool operator()(const nil) const
  {
    return false;
  }

  bool operator()(unsigned int) const
  {
    return false;
  }

  bool operator()(float) const
  {
    return false;
  }

  bool operator()(const attribute &a) const
  {
    bool result = false;
    for (auto path_element: a.attribute_path) {
      result |= boost::apply_visitor(*this, path_element);
    }
    return result;
  }

  bool operator()(const attribute_path_subscript &subscript) const
  {
    bool result = false;
    result |= boost::apply_visitor(*this, subscript.identifier);
    result |= boost::apply_visitor(*this, subscript.index);
    return result;
  }

  bool operator()(const operand &operand) const
  {
    return boost::apply_visitor(*this, operand);
  }

  bool operator()(const signed_ &x) const
  {
    return boost::apply_visitor(*this, x.operand_);
  }

  bool operator()(const condition &x) const
  {
    bool result = false;
    result |= boost::apply_visitor(*this, x.cond_value);
    result |= boost::apply_visitor(*this, x.cond_true);
    result |= boost::apply_visitor(*this, x.cond_false);
    return result;
  }

  bool operator()(const operation &x) const
  {
    return boost::apply_visitor(*this, x.operand_);
  }

  bool operator()(const expression &x) const
  {
    bool result = boost::apply_visitor(*this, x.first);
    for (const auto &terms: x.rest) {
      result |= (*this)(terms);
    }
    return result;
  }

  bool operator()(const assignment &a) const
  {
    // We assume no functions on the left hand side of assignments.
    bool result = (*this)(a.rhs);
    return result;
  }

  bool operator()(const function_invokation &f) const
  {
    const auto &functions = get_built_in_functions<float>();
    if (functions.count(f.function_name) == 0) {
      sl_log_error("attribute_mapper_built_in_function_name_check",
                   "Unknown function (%s) found in AST tree",
                   f.function_name.c_str());
      return true;
    }

    // Check for the function arguments, they may be functions themselves.
    for (auto &argument: f.arguments) {
      if (true == (*this)(argument)) {
        return true;
      }
    }

    return false;
  }

  bool operator()(const scope &x) const
  {
    bool result = false;
    for (auto &a: x.assignments) {
      result |= (*this)(a);
    }
    return result;
  }

  bool operator()(const ast_tree &ast) const
  {
    bool result = false;
    for (const auto &ast_element: ast) {
      result |= boost::apply_visitor(*this, ast_element);
    }
    return result;
  }
};

}  // namespace ast

#ifdef __cplusplus
extern "C" {
#endif

/** @} end attribute_mapper_built_in_functions */
#ifdef __cplusplus
}
#endif

#endif  //ATTRIBUTE_MAPPER_BUILT_IN_FUNCTIONS_HPP
