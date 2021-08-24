/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @file attribute_mapper_ast.hpp
 * @brief Attribute mapper ast
 *
 * The datastructures defined below are all the structures which makes up
 * the atttribute mapping ast. All UAM files are parsed into theese
 * structures.
 *
 * @{
 */

#ifndef ATTRIBUTE_MAPPER_AST_HPP
#define ATTRIBUTE_MAPPER_AST_HPP

#include <vector>
#include <list>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

namespace ast
{
namespace x3 = boost::spirit::x3;

struct nil {};
struct signed_;
struct expression;
struct condition;
struct attribute;
struct attribute_path_subscript;
struct assignment;
struct scope;

using ast_node = boost::variant<ast::scope>;
struct ast_tree : public std::vector<ast_node> {};

/**
 * @brief Definitions of operators
 * When the parser encounters the string representation of
 * operators, they are parsed into one of the following enums.
 */
enum operator_ids {
  operator_plus,
  operator_minus,
  operator_comp,
  operator_mult,
  operator_div,
  operator_bitand,
  operator_bitor,
  operator_bitxor,
  operator_left_shift,
  operator_right_shift,
  operator_or,
};

/**
 * @brief Operands
 *
 * Example &lt;operand1&gt; + &lt;operand2&gt;,
 *
 * An operand can be made up of various elements. This structure
 * holds the data of one of the operand types together with information
 * about which type the objects actually is.
 */
using operand = x3::variant<nil,
                            unsigned int,
                            x3::forward_ast<attribute>,
                            x3::forward_ast<signed_>,
                            x3::forward_ast<expression>,
                            x3::forward_ast<condition>>;
/**
 * @brief Attribute path element
 *
 * This describes one element in an attribute path. This can either be
 * an operand ( like a integer constant ) or a subscript ( 1234[4] )
 *
 */
using attribute_path_element
  = boost::variant<ast::operand, unsigned int, attribute_path_subscript>;

/**
 * @brief Uniary sigend operand
 *
 * Examples -1 , +2
 */
struct signed_ {
  char sign;
  operand operand_;
};

/**
 * @brief Operation
 * An operation consists of an operator and an right
 * hand side operand. When operations are evaluated they
 * are always evaluated in the context of left hand side
 * value.
 */
struct operation {
  operator_ids operator_;  ///< +,-,*,/ etc.
  operand operand_;        ///< right hand side operand
};

/**
 * @brief Expression
 *
 * An expression consist of one or more terms seperated by a weak
 * binding operator.
 */
struct expression {
  operand first;
  std::list<operation> rest;  ///< + or - operations
};

/**
 * @brief Condition
 *
 * This is an if statement.
 *
 * Example
 * if 1 2 3
 */
struct condition {
  operand cond_value;  ///< Selector value
  operand cond_true;   ///< Truth value
  operand cond_false;  ///< False value
};

/**
 * @brief Attribute path subscript
 *
 * used to hold information about path subscripting
 *
 * example: 1245[4]
 */
struct attribute_path_subscript {
  operand identifier;
  operand index;
};

/**
 * @brief Attribute
 *
 * This is the representation of an attribute. The type can be
 * r: reported
 * d: desired
 * e: exists
 *
 * The attribute path consits of atleast one path element
 */
struct attribute {
  char value_type;
  std::vector<attribute_path_element> attribute;
};

/**
 * @brief Assignment
 *
 * This holds the data for an attribute assignment
 *
 * Example:
 *
 * r'53422 = r'321 + 3
 *
 */
struct assignment {
  ast::attribute attribute;  ///< left hand side of the assignment
  expression rhs;            ///< right hand side of the assignment
};

/**
 * @brief Scope
 *
 * This defines a mappng scope with a priority
 * which may hold multiple assignments.
 */
struct scope {
  int priority;
  std::vector<assignment> assignments;
};

bool operator==(const nil &a, const nil &b);
bool operator==(const operand &a, const operand &b);
bool operator==(const signed_ &a, const signed_ &b);
bool operator==(const operation &a, const operation &b);
bool operator==(const expression &a, const expression &b);
bool operator==(const condition &a, const condition &b);
bool operator==(const attribute &a, const attribute &b);
bool operator==(const attribute_path_subscript &a,
                const attribute_path_subscript &b);
bool operator==(const assignment &a, const assignment &b);

}  // namespace ast

/**
 * @brief Adapaters for boost fusion.
 *
 * The macros below define adaptor functions which allows the
 * ast struct to be used we boost fusion, which the parser needs
 * for put the parsed tokens into the structs defined above.
 *
 */
BOOST_FUSION_ADAPT_STRUCT(ast::signed_, sign, operand_)
BOOST_FUSION_ADAPT_STRUCT(ast::operation, operator_, operand_)
BOOST_FUSION_ADAPT_STRUCT(ast::expression, first, rest)
BOOST_FUSION_ADAPT_STRUCT(ast::assignment, attribute, rhs)
BOOST_FUSION_ADAPT_STRUCT(ast::condition, cond_value, cond_true, cond_false)
BOOST_FUSION_ADAPT_STRUCT(ast::attribute, value_type, attribute)
BOOST_FUSION_ADAPT_STRUCT(ast::attribute_path_subscript, identifier, index)
BOOST_FUSION_ADAPT_STRUCT(ast::scope, priority, assignments)

#endif  //ATTRIBUTE_MAPPER_AST_HPP

/**
 * @}
 */
