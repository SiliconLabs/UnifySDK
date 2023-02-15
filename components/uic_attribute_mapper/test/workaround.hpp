#ifndef WORKAROUND_HPP
#define WORKAROUND_HPP

/* This is a workaround to hide the C++ includes from
 * Unity's generate_test_runner.rb
 */

#ifdef __cplusplus
#include "attribute_mapper_engine.hpp"
#include "attribute_mapper_parser.hpp"
#include "attribute_mapper_ast_eval.hpp"
#include "attribute_mapper_ast_reducer.hpp"
#include "attribute_mapper_ast_print.hpp"
#include "attribute_mapper_ast_complexity.hpp"
#include "attribute_mapper_built_in_functions.hpp"
#include "attribute_store_test_helpers.hpp"
#include "smartstart_management.hpp"
#endif
#endif
