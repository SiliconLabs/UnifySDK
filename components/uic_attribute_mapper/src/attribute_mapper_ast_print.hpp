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
#include "attribute_store_type_registration.h"

/**
 * @defgroup attribute_mapper_ast_print Abstract Syntax Tree Print
 * @ingroup unify_attribute_mapper
 * @brief Write a description about this
 *
 * @{
 */

namespace ast
{
class print
{
  public:
  print(std::ostream &out) : _out(out) {};

  inline void operator()(const nil &x)
  {
    _out << "undefined";
  }

  inline void operator()(float n)
  {
    _out << n;
  }

  inline void operator()(uint32_t n)
  {
    attribute_store_type_t type = (attribute_store_type_t)round(n);
    if (resolve_attr_names && attribute_store_is_type_registered(type)) {
      const char *name = attribute_store_get_type_name(type);
      if (name) {
        _out << name;
        return;
      }
    }
    _out << std::hex << "0x" << type;
  }

  inline void operator()(const attribute &a);
  inline void operator()(const signed_ &x)
  {
    _out << x.sign;
    boost::apply_visitor(*this, x.operand_);
  }

  inline void operator()(const condition &x)
  {
    _out << "if ";
    boost::apply_visitor(*this, x.cond_value);
    _out << ' ';
    boost::apply_visitor(*this, x.cond_true);
    _out << ' ';
    boost::apply_visitor(*this, x.cond_false);
  }

  inline void operator()(const operation &x)
  {
    const char *op_str[] = {"+",
                            "-",
                            "==",
                            "!=",
                            "*",
                            "/",
                            "&",
                            "|",
                            "^",
                            "<",
                            ">",
                            "or",
                            "%",
                            "**"};
    _out << op_str[x.operator_] << ' ';
    boost::apply_visitor(*this, x.operand_);
  }

  inline void operator()(const expression &x)
  {
    _out << "( ";
    boost::apply_visitor(*this, x.first);
    _out << ' ';
    for (auto terms: x.rest) {
      (*this)(terms);
      _out << ' ';
    }
    _out << ")";
  }

  inline void operator()(const assignment &a)
  {
    (*this)(a.lhs);
    _out << " = ";
    (*this)(a.rhs);
  }

  inline void operator()(const scope &x)
  {
    _out << "scope " << x.priority << " {" << std::endl;
    for (auto &a: x.assignments) {
      _out << "  ";
      (*this)(a);
      _out << std::endl;
    }
    _out << "}" << std::endl;
  }

  inline void operator()(const ast_tree ast)
  {
    for (const auto &ast_element: ast) {
      boost::apply_visitor(*this, ast_element);
    }
  }

  bool resolve_attr_names = false;

  private:
  std::ostream &_out;
};

class path_printer
{
  public:
  path_printer(std::ostream &out) : _out(out), _printer(out) {}

  inline void operator()(const ast::operand &operand)
  {
    _printer.resolve_attr_names = true;
    boost::apply_visitor(_printer, operand);
    _printer.resolve_attr_names = false;
  }
  // hat operator ^ (parent)
  inline void operator()(const nil &nul)
  {
    _out << '^';
  }
  // just given by type id
  inline void operator()(result_type_t type_id)
  {
    _out << round(type_id);
  }

  // Subscript operator
  inline void operator()(const attribute_path_subscript &subscript)
  {
    _printer.resolve_attr_names = true;
    boost::apply_visitor(_printer, subscript.identifier);
    _printer.resolve_attr_names = false;
    _out << '[';
    boost::apply_visitor(_printer, subscript.index);
    _out << ']';
  }
  // parse a path list
  inline void operator()(const std::vector<attribute_path_element> &paths)
  {
    for (auto it = paths.begin(); it != paths.end(); it++) {
      boost::apply_visitor(*this, *it);
      if ((it + 1) != paths.end()) {
        _out << '.';
      }
    }
  }

  private:
  std::ostream &_out;
  print _printer;
};

inline void print::operator()(const attribute &a)
{
  _out << a.value_type << '\'';
  path_printer pp(_out);
  pp(a.attribute_path);
}

/************************ ostream helpers *************************/

inline std::ostream &operator<<(std::ostream &s,
                                const attribute_dependency_t &d)
{
  s << d.second << "'" << attribute_store_get_type_name(d.first);
  return s;
}

inline std::ostream &operator<<(std::ostream &s, const assignment &d)
{
  print printer(s);
  printer(d);
  return s;
}

inline std::ostream &operator<<(std::ostream &s, const attribute &d)
{
  print printer(s);
  printer(d);
  return s;
}

inline std::ostream &operator<<(std::ostream &s, const ast_tree &d)
{
  print printer(s);
  printer(d);
  return s;
}

}  // namespace ast

/** @} end attribute_mapper_ast_print */
