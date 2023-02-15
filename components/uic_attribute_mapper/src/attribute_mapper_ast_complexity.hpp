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

#include <iostream>
#include <iomanip>
#include "attribute.hpp"
#include "attribute_mapper_ast.hpp"

/**
 * @defgroup attribute_mapper_ast_complexity Abstract Syntax Tree complexity counter
 * @ingroup unify_attribute_mapper
 * @brief This ast visitor counts the complexity of the ast tree, but counting the
 * number of nodes in the tree
 *
 * @{
 */

namespace ast
{
class complexity
{
  public:
  inline int operator()(const nil &x)
  {
    return 1;
  }

  inline int operator()(unsigned int n)
  {
    return 1;
  }

  inline int operator()(float n)
  {
    return 1;
  }

  inline int operator()(const attribute &a);
  inline int operator()(const signed_ &x)
  {
    return boost::apply_visitor(*this, x.operand_) + 1;
  }

  inline int operator()(const condition &x)
  {
    int n = 0;
    n += boost::apply_visitor(*this, x.cond_value);
    n += boost::apply_visitor(*this, x.cond_true);
    n += boost::apply_visitor(*this, x.cond_false);
    return n;
  }

  inline int operator()(const operation &x)
  {
    return boost::apply_visitor(*this, x.operand_) + 1;
  }

  inline int operator()(const expression &x)
  {
    int n = boost::apply_visitor(*this, x.first);
    for (const auto &terms: x.rest) {
      n += (*this)(terms);
    }
    return n;
  }

  inline int operator()(const assignment &a)
  {
    int n = 0;
    n += (*this)(a.lhs);
    n += (*this)(a.rhs);
    return n;
  }

  inline int operator()(const function_invokation &f)
  {
    int n = 0;
    for (auto &argument: f.arguments) {
      n += (*this)(argument);
    }
    return n;
  }

  inline int operator()(const scope &x)
  {
    int n = 0;
    for (auto &a: x.assignments) {
      n += (*this)(a);
    }
    return n;
  }

  inline int operator()(const ast_tree ast)
  {
    int n = 0;
    for (const auto &ast_element: ast) {
      n += boost::apply_visitor(*this, ast_element);
    }
    return n;
  }
};

class path_complexity
{
  public:
  path_complexity(complexity &complexity) : _complexity(complexity) {}

  inline int operator()(const ast::operand &operand)
  {
    return boost::apply_visitor(_complexity, operand) + 1;
  }
  // hat operator ^ (parent)
  inline int operator()(const nil &nul)
  {
    return 1;
  }
  // just given by type id
  inline int operator()(attribute_store_type_t type)
  {
    return 1;
  }

  // Subscript operator
  inline int operator()(const attribute_path_subscript &subscript)
  {
    int n = 0;
    n += boost::apply_visitor(_complexity, subscript.identifier);
    n += boost::apply_visitor(_complexity, subscript.index);
    return n;
  }
  // parse a path list
  inline int operator()(const std::vector<attribute_path_element> &paths)
  {
    int n = 0;
    for (auto it = paths.begin(); it != paths.end(); it++) {
      n += boost::apply_visitor(*this, *it);
    }
    return n;
  }

  private:
  complexity &_complexity;
};

inline int complexity::operator()(const attribute &a)
{
  path_complexity pc(*this);
  return pc(a.attribute_path) + 1;
}

}  // namespace ast

/** @} end attribute_mapper_ast_print */
