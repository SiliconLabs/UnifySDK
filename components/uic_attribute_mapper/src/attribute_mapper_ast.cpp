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

#include "attribute_mapper_ast.hpp"
///////////////////////////////////////////////////////////////////////////////
// Uncomment this if you want to enable debugging
//#define BOOST_SPIRIT_X3_DEBUG

namespace ast
{
bool operator==(const nil &a, const nil &b)
{
  return true;
}

bool operator==(const operand &a, const operand &b)
{
  return a.get() == b.get();
}

bool operator==(const signed_ &a, const signed_ &b)
{
  return a.sign == b.sign && a.operand_ == b.operand_;
}

bool operator==(const operation &a, const operation &b)
{
  return a.operator_ == b.operator_ && a.operand_ == b.operand_;
}

bool operator==(const expression &a, const expression &b)
{
  return a.first == b.first && a.rest == b.rest;
}

bool operator==(const condition &a, const condition &b)
{
  return a.cond_value == b.cond_value && a.cond_true == b.cond_true
         && a.cond_false == b.cond_false;
}

bool operator==(const attribute &a, const attribute &b)
{
  return a.value_type == b.value_type && a.attribute_path == b.attribute_path;
}

bool operator==(const attribute_path_subscript &a,
                const attribute_path_subscript &b)
{
  return a.identifier == b.identifier && a.index == b.index;
}

bool operator==(const assignment &a, const assignment &b)
{
  return a.lhs == b.lhs && a.rhs == b.rhs;
}

}  // namespace ast
