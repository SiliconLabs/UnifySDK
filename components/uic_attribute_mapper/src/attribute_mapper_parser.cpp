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

#include "attribute_mapper_grammar.hpp"
#include "attribute_mapper_ast.hpp"

namespace ast
{
bool Parse(const std::string &file_to_parse, ast_tree &ast)
{
  using IteratorType = std::string::const_iterator;
  using skipperType  = SkipperGrammar<IteratorType>;
  using grammarType  = UAMGrammar<IteratorType, skipperType>;

  static grammarType grammar;
  static skipperType skipper;

  IteratorType begin(file_to_parse.begin());
  IteratorType end(file_to_parse.end());

  if (!phrase_parse(begin, end, grammar, skipper, ast)) {
    std::cout << "parser error" << std::endl;
  }

  if (begin != end) {
    std::cout << "Did not pase to the end of file." << '\n';
    return false;
  }

  return true;
}

}  // namespace ast
