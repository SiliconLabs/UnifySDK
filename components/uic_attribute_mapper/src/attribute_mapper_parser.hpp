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
 * @defgroup attribute_mapper_parser Mapper parser
 * @ingroup unify_attribute_mapper
 *
 * @brief Parses strings/text into Abtract Syntax Trees
 *
 * @{
 */

#ifndef ATTRIBUTE_MAPPER_PARSER_HPP
#define ATTRIBUTE_MAPPER_PARSER_HPP

#include <string>

namespace ast
{
struct ast_tree;
/**
 * @brief Parse UAM script and build and AST
 *
 * This function is the entry point of the parser, but actual parser logic is
 * defined in attribute_mapper_grammar.hpp
 *
 * @param string_to_parse String to parse
 * @param ast Output AST
 * @return true If parse was successfull.
 * @return false  False is parse did not succeed.
 */
bool Parse(const std::string &string_to_parse, ast::ast_tree &ast);

}  // namespace ast

#endif
/** @} end attribute_mapper_parser */
