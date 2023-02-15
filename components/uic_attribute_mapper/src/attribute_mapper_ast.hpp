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

/**
 * @defgroup attribute_mapper_ast Abstract Syntax Tree
 * @ingroup unify_attribute_mapper
 * @brief Attribute Mapper Abstract Syntax Tree
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
#include <boost/spirit/include/qi_optional.hpp>

using result_type_t = float;

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
struct function_invokation;
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
  operator_equals,
  operator_mult,
  operator_div,
  operator_bitand,
  operator_bitor,
  operator_bitxor,
  operator_less_than,
  operator_greater_than,
  operator_or,
  operator_modulo,
  operator_exponent,
  operator_neq,
  operator_less_than_or_eq,
  operator_greater_than_or_eq,
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
using operand = x3::variant<ast::nil,
                            uint32_t,
                            float,
                            x3::forward_ast<attribute>,
                            x3::forward_ast<signed_>,
                            x3::forward_ast<expression>,
                            x3::forward_ast<condition>,
                            x3::forward_ast<function_invokation>>;
/**
 * @brief Attribute path element
 *
 * This describes one element in an attribute path. This can either be
 * an operand ( like a integer constant ) or a subscript ( 1234[4] )
 *
 */
using attribute_path_element
  = boost::variant<ast::nil, ast::operand, uint32_t, attribute_path_subscript>;

/**
 * @brief Uniary signed operand
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
  std::vector<attribute_path_element> attribute_path;
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
  ast::attribute lhs;  ///< left hand side of the assignment
  expression rhs;      ///< right hand side of the assignment
};

/**
 * @brief Built-in function invokation
 * Example : fn_min_value(r'3, 5, d'5 or 20)
 */
struct function_invokation {
  std::string function_name;          // Function name to invoke.
  std::vector<expression> arguments;  // Arguments for the function
};

/**
 * @brief Scope setting
 * A scope setting consists of a setting name and an associated value
 */
struct scope_setting {
  std::string setting_name;
  boost::optional<unsigned int> setting_value;
};

/**
 * @brief Type for Scope settings list
 */
using scope_settings_t = std::vector<scope_setting>;

/**
 * @brief Scope
 *
 * This defines a mappng scope with a priority
 * which may hold multiple assignments.
 */
struct scope {
  int priority;
  scope_settings_t settings;
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
bool operator==(const scope_setting &a, const scope_setting &b);
bool operator==(const function_invokation &a, const function_invokation &b);

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
BOOST_FUSION_ADAPT_STRUCT(ast::assignment, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(ast::condition, cond_value, cond_true, cond_false)
BOOST_FUSION_ADAPT_STRUCT(ast::attribute, value_type, attribute_path)
BOOST_FUSION_ADAPT_STRUCT(ast::attribute_path_subscript, identifier, index)
BOOST_FUSION_ADAPT_STRUCT(ast::function_invokation, function_name, arguments)
BOOST_FUSION_ADAPT_STRUCT(ast::scope_setting,
                          (std::string, setting_name),
                          (boost::optional<unsigned int>, setting_value))
BOOST_FUSION_ADAPT_STRUCT(ast::scope, priority, settings, assignments)

#endif  //ATTRIBUTE_MAPPER_AST_HPP

/**
 * @}
 */
