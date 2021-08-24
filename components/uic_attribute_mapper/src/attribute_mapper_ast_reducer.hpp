#ifndef ATTRIBUTE_MAPPER_AST_REDUCER_H
#define ATTRIBUTE_MAPPER_AST_REDUCER_H

#include "attribute_mapper_ast.hpp"
#include "attribute_mapper_ast_eval.hpp"

namespace ast
{
struct ast_tree;
// postorder tree traversal, build new tree, with the following cases reduces:
// - if expression contains 2 or more constants but not all. they will condensed in one
// - if all children of a expression contain a constanst replace root expression with constant
void reduce_tree(ast_tree &);

class reduce_visitor : public boost::static_visitor<>
{
  public:
  void operator()(ast::ast_tree &tree)
  {
    for (auto &root: tree) {
      boost::apply_visitor(*this, root);
    }
  }

  void operator()(expression &x)
  {
    // // for (const auto &op: x.rest) {
    // //   boost::apply_visitor(*this, op.operand_);
    // // }

    // program_visitor visitor;
    // boost::apply_visitor(visitor, x);
    // x = visitor.GetOptimizedProgram();
    // // Todo do stuff
  }

  void operator()(nil &n)
  {
    // omit, no operation
  }

  void operator()(unsigned int &n)
  {
    constant_program.rest.emplace_back(n);
  }

  void operator()(attribute &attr)
  {
    // not implemented yet
  }

  void operator()(signed_ &sign)
  {
    constant_program.rest.emplace_back(sign);
  }

  void operator()(condition &con)
  {
    // program_visitor visitor;
    // boost::apply_visitor(visitor, con.cond_value);
    // if (visitor.IsConstant()) {
    // }
  }

  void operator()(mapping &map)
  {
    // ignore
  }

  void operator()(assignment &map)
  {
    // ignore
  }

  expression &GetOptimizedProgram()
  {
    // todo
    return constant_program;
  }

  bool IsConstant()
  {
    return false;
    //return new_children.empty() && constant_program.rest.size() > 0;
  }

  private:
  expression constant_program;
  //std::list<operation> new_children;
};

void reduce_tree(ast_tree &tree)
{
  reduce_visitor visitor;
  visitor(tree);
  visitor.IsConstant();
}

}  // namespace ast

#endif
