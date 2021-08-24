

#include <iostream>
#include <iomanip>
#include "attribute.hpp"
#include "attribute_mapper_ast.hpp"
#include "attribute_store_debug.h"
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
      = {"+", "-", "==", "*", "/", "&", "|", "^", "<<", ">>", "or"};
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
    (*this)(a.attribute);
    _out << " = ";
    (*this)(a.rhs);
  }

  bool resolve_attr_names;

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
  pp(a.attribute);
}

}  // namespace ast