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
#include "attribute_store_debug.h"

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

  void operator()(const nil &x)
  {
    _out << "undefined";
  }

  void operator()(unsigned int n)
  {
    if (resolve_attr_names) {
      const char *name = attribute_store_name_by_type(n);
      if (name) {
        _out << name;
        return;
      }
    }
    _out << "0x" << std::setw(8) << std::setfill('0') << std::hex << n;
  }

  void operator()(const attribute &a);
  void operator()(const signed_ &x)
  {
    _out << x.sign;
    boost::apply_visitor(*this, x.operand_);
  }

  void operator()(const condition &x)
  {
    _out << "if ";
    boost::apply_visitor(*this, x.cond_value);
    _out << ' ';
    boost::apply_visitor(*this, x.cond_true);
    _out << ' ';
    boost::apply_visitor(*this, x.cond_false);
  }

  void operator()(const operation &x)
  {
    const char *op_str[]
      = {"+", "-", "==", "*", "/", "&", "|", "^", "<", ">", "or"};
    _out << op_str[x.operator_] << ' ';
    boost::apply_visitor(*this, x.operand_);
  }

  void operator()(const expression &x)
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

  void operator()(const assignment &a)
  {
    (*this)(a.lhs);
    _out << " = ";
    (*this)(a.rhs);
  }

  bool resolve_attr_names = false;

  private:
  std::ostream &_out;
};

class path_printer
{
  public:
  path_printer(std::ostream &out) : _out(out), _printer(out) {}

  void operator()(const ast::operand &operand)
  {
    _printer.resolve_attr_names = true;
    boost::apply_visitor(_printer, operand);
    _printer.resolve_attr_names = false;
  }
  // hat operator ^ (parent)
  void operator()(const nil &nul)
  {
    _out << '^';
  }
  // just given by type id
  void operator()(unsigned int type_id)
  {
    _out << type_id;
  }

  // Subscript operator
  void operator()(const attribute_path_subscript &subscript)
  {
    _printer.resolve_attr_names = true;
    boost::apply_visitor(_printer, subscript.identifier);
    _printer.resolve_attr_names = false;
    _out << '[';
    boost::apply_visitor(_printer, subscript.index);
    _out << ']';
  }
  // parse a path list
  void operator()(const std::vector<attribute_path_element> &paths)
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

void print::operator()(const attribute &a)
{
  _out << a.value_type << '\'';
  path_printer pp(_out);
  pp(a.attribute_path);
}

/************************ ostream helpers *************************/

inline std::ostream &operator<<(std::ostream &s,
                                const attribute_dependency_t &d)
{
  s << d.second << "'" << attribute_store_name_by_type(d.first);
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

}  // namespace ast

/** @} end attribute_mapper_ast_print */
