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
 * @defgroup attribute_mapper_grammar Mapper grammar
 * @ingroup unify_attribute_mapper
 *
 * @brief describes the grammar rules of .uam files
 *
 * This file contains the definition of two grammes
 * which are used by the mail UAM parser.
 *
 * A skipper parse which parses whitspaces and comments
 * and a UAM Grammer parser.
 *
 * @{
 */

#ifndef ATTRIBUTE_MAPPER_GRAMMAR_H
#define ATTRIBUTE_MAPPER_GRAMMAR_H

#include <boost/any.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_grammar.hpp>
#include <boost/phoenix/operator.hpp>
#include <boost/phoenix/object.hpp>
#include <boost/phoenix/bind.hpp>
#include "attribute_mapper_ast.hpp"

namespace qi = boost::spirit::qi;

// Operators in the term rule have higher precedence than
// those in expression rule.
struct term_rule_operators_ : qi::symbols<char, ast::operator_ids> {
  term_rule_operators_()
  {
    // clang-format off
    add("*",  ast::operator_ids::operator_mult)
       ("/",  ast::operator_ids::operator_div)
       ("<",  ast::operator_ids::operator_left_shift)
       (">",  ast::operator_ids::operator_right_shift)
       ("|",  ast::operator_ids::operator_bitor)
       ("&",  ast::operator_ids::operator_bitand)
       ("^",  ast::operator_ids::operator_bitxor)
       ("or", ast::operator_ids::operator_or)
       ("%",  ast::operator_ids::operator_modulo)
       ("**", ast::operator_ids::operator_exponent);
    // clang-format on
  }
} term_rule_operators;

// Operators in the expression rule have lower precedence than
// those in the term rule.
struct expr_rule_operators_ : qi::symbols<char, ast::operator_ids> {
  expr_rule_operators_()
  {
    // clang-format off
    add("+",    ast::operator_ids::operator_plus)
       ("-",    ast::operator_ids::operator_minus)
       ("==",   ast::operator_ids::operator_equals)
       ("!=",   ast::operator_ids::operator_neq);
    // clang-format on
  }
} expr_rule_operators;

/**
 * @brief Skipper grammer
 *
 * This parser parses whitespaces and comments.
 * The skipper does not produce any output.
 *
 * @tparam IteratorType
 */
template<typename IteratorType> class SkipperGrammar :
  public qi::grammar<IteratorType>
{
  public:
  SkipperGrammar() : SkipperGrammar::base_type(rule)
  {
    lineCommentRule = qi::lit("//") >> *(qi::char_ - qi::eol) >> qi::eol;
    blockCommentRule
      = qi::lit("/*") >> *(qi::char_ - qi::lit("*/")) >> qi::lit("*/");
    spaceRule = qi::space;
    rule      = spaceRule | lineCommentRule | blockCommentRule;
  }

  private:
  qi::rule<IteratorType> lineCommentRule;
  qi::rule<IteratorType> blockCommentRule;
  qi::rule<IteratorType> spaceRule;
  qi::rule<IteratorType> rule;
};

/**
 * @brief Main UAM grammer
 *
 * This class defines the UAM grammer and is closely coupled with data types
 * in the AST. This class does not in it self produce anything but a grammer
 * definition, but when used with the boost::qi:parse_phrase function a text document can
 * be transformed into a AST data structure.
 *
 * For details on how this works see
 * https://www.boost.org/doc/libs/1_76_0/libs/spirit/doc/html/spirit/qi/tutorials/employee___parsing_into_structs.html
 *
 * @tparam IteratorType
 * @tparam Skipper
 */

template<typename T> struct strict_real_policies :
  boost::spirit::qi::real_policies<T> {
  static bool const expect_dot = true;
};

template<typename IteratorType, typename Skipper> class UAMGrammar :
  public qi::grammar<IteratorType, ast::ast_tree(), Skipper>
{
  public:
  UAMGrammar() : UAMGrammar::base_type(start_rule, "Rules")
  {
    using qi::alnum;
    using qi::alpha;
    using qi::char_;
    using qi::hex;
    using qi::uint_;

    using qi::fail;
    using qi::on_error;
    using namespace qi::labels;
    using boost::phoenix::construct;
    using boost::phoenix::val;
    using boost::spirit::qi::real_parser;
    using boost::spirit::qi::strict_ureal_policies;
    //This parser parses real numbers, real numbers must contain a dot
    real_parser<float, strict_ureal_policies<float>> ureal;
    // definitions, example: def my_var 1234
    // note that here we use a schematic action to build the defs parser.
    // in this way when the parse later passes an identifier the defs parser
    // will output the expression from the def_rule.
    def_rule
      = "def" > (identifier_rule
                 > expression_rule)[boost::phoenix::bind(defs.add, _1, _2)];

    // identifier, name of something
    identifier_rule = (alpha | char_("_")) >> *(alnum | char_("_"));

    // elements of an attribute path
    attribute_path_element_rule
      = '^' | (operand_rule >> '[' >> operand_rule >> ']') | operand_rule;

    // the whole attribute path, example d'1245.6666[3].9999
    attribute_rule = char_("dre") >> '\'' >> attribute_path_element_rule % '.';

    // if condition, example if (r'1244 == 0) 2 4
    condition_rule = "if" > operand_rule > operand_rule > operand_rule;

    // any operand type, numbers, attributes conditions etc.
    operand_rule = "undefined" | ("0x" > hex) | (ureal >> 'f') | uint_
                   | attribute_rule | condition_rule | defs
                   | ('(' > expression_rule > ')') | (char_('-') > operand_rule)
                   | (char_('+') > operand_rule);

    // a term is made of 1 or more operators combined by a strong binding operator like * / | &
    term_rule = operand_rule >> *(term_rule_operators > operand_rule);

    // an experssion is made of 1 or more operators combined by a weak binding operator like + and -
    expression_rule = (term_rule >> *(expr_rule_operators > term_rule));

    // an assignment, example r'1234 = r'6666
    assignment_rule = attribute_rule >> "=" >> expression_rule;

    // the scope rule is used to group assignments and assign them a priority.
    scope_rule = "scope" > uint_ >> "{" >> +assignment_rule >> "}";

    // This is the entry rule for the grammer, definitions must come first, then scopes
    start_rule = *def_rule > *scope_rule;

    expression_rule.name("expression");
    term_rule.name("term");
    operand_rule.name("operand");
    condition_rule.name("condition");
    attribute_rule.name("attribute");
    assignment_rule.name("assignment");
    scope_rule.name("scope");
    def_rule.name("def");
    identifier_rule.name("identifier");
    attribute_path_element_rule.name("path element");

    // Error handler for the parser
    // See https://www.boost.org/doc/libs/1_76_0/libs/spirit/doc/html/spirit/qi/tutorials/mini_xml___error_handling.html
    on_error<fail>(
      start_rule,
      std::cout << val("Error! Expecting ") << _4  // what failed?
                << val(" here: \"")
                << construct<std::string>(_3,
                                          _2)  // iterators to error-pos, end
                << val("\"") << std::endl);
  }

  private:
  qi::symbols<char, ast::expression> defs;

  // Rules ( small parsers ) used by the main parser.
  // note that the second template argument is the data type of the output of the
  // rule. The tokens produced by the parser must be compatible destination struct
  // Ie if a rule parses and integer followed by a string then the struct in the
  // second argument must have a integer as first argument and a string as second argument.
  // See documentation for details:
  // https://www.boost.org/doc/libs/1_76_0/libs/spirit/doc/html/spirit/qi/tutorials/employee___parsing_into_structs.html
  qi::rule<IteratorType, ast::expression(), Skipper> expression_rule;
  qi::rule<IteratorType, ast::expression(), Skipper> term_rule;
  qi::rule<IteratorType, ast::operand(), Skipper> operand_rule;
  qi::rule<IteratorType, ast::condition(), Skipper> condition_rule;
  qi::rule<IteratorType, ast::attribute(), Skipper> attribute_rule;
  qi::rule<IteratorType, ast::attribute_path_element(), Skipper>
    attribute_path_element_rule;
  qi::rule<IteratorType, std::string()> identifier_rule;
  qi::rule<IteratorType, ast::assignment(), Skipper> assignment_rule;
  qi::rule<IteratorType, ast::scope(), Skipper> scope_rule;
  qi::rule<IteratorType, Skipper> def_rule;
  qi::rule<IteratorType, ast::ast_tree(), Skipper> start_rule;
};

#endif  //ATTRIBUTE_MAPPER_GRAMMAR_H

/**
 * @}
 */
