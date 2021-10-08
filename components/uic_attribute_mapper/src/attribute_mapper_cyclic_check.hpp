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
 * @file attribute_mapper_cyclic_check.hpp
 * @brief Component cyclic dependency checker
 *
 * @{
 */

#ifndef ATTRIBUTE_MAPPER_CYCLIC_CHECK_HPP
#define ATTRIBUTE_MAPPER_CYCLIC_CHECK_HPP

#include "attribute_mapper_ast.hpp"
#include "attribute_mapper_ast_eval.hpp"
#include "attribute_mapper_ast_dep_eval.hpp"

namespace ast
{
/**
 * @brief Check if a list of relations contain cyclic dependencies.
 * 
 * @param relations reference to relation table
 * @param ast 
 *
 * @return true cyclic dependencies are detected
 * @return false cyclic dependencies are not detected.
 */
bool check_cyclic_dependencies(
  const ast::ast_tree &ast,
  const std::multimap<ast::attribute_dependency_t, ast::assignment> &relations);
}  // namespace ast

#endif  //ATTRIBUTE_MAPPER_CYCLIC_CHECK_HPP
/** @} end attribute_mapper_cyclic_check */
