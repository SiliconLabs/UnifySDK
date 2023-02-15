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
#include "attribute_mapper_engine.hpp"

#include "attribute_mapper.h"
#include "attribute_mapper_process.h"
#include "attribute_mapper_parser.hpp"
#include "attribute_mapper_ast_print.hpp"
#include "attribute_mapper_ast_eval.hpp"
#include "attribute_mapper_ast_dep_eval.hpp"
#include "attribute_mapper_cyclic_check.hpp"
#include "attribute_mapper_ast_reducer.hpp"
#include "attribute_mapper_built_in_functions.hpp"
#include "attribute_mapper_scope_settings.hpp"

#include "sl_log.h"
#include <map>
#include <fstream>
#include <assert.h>
#include <boost/filesystem.hpp>

constexpr const char LOG_TAG[] = "attribute_mapper";

void MapperEngine::reset()
{
  relations.clear();
  assignment_settings.clear();
}

bool MapperEngine::load_path(const std::string &uam_path)
{
  using namespace boost::filesystem;

  std::vector<std::string> uam_files;

  if (!is_directory(uam_path)) {
    sl_log_error(LOG_TAG,
                 "Directory containing UAM files could not be found: %s.",
                 uam_path.data());
    return false;
  }
  for (auto &p: directory_iterator(uam_path)) {
    //      sl_log_info(LOG_TAG, "file %s",p.path().extension().c_str());
    if (is_regular_file(p.path()) && p.path().extension() == ".uam") {
      uam_files.push_back(p.path().string());
    }
  }

  //Read all mapping files
  for (auto it = uam_files.begin(); it != uam_files.end(); it++) {
    if (!load_file(*it)) {
      return false;
    }
  }

  return true;
}

bool MapperEngine::load_file(std::string uam_file_name)
{
  using namespace boost::filesystem;

  if (!is_regular_file(uam_file_name)) {
    sl_log_error(LOG_TAG,
                 "Specified UAM mapping file %s is invalid.",
                 uam_file_name.c_str());
    return false;
  }

  //Read mapping file
  sl_log_debug(LOG_TAG, "Loading UAM mapping file %s", uam_file_name.c_str());

  std::ifstream uam_file(uam_file_name);
  if (!uam_file) {
    sl_log_error(LOG_TAG,
                 "Could not open mapping file %s.",
                 uam_file_name.c_str());
    return false;
  }
  std::string str((std::istreambuf_iterator<char>(uam_file)),
                  std::istreambuf_iterator<char>());
  ast::ast_tree ast;
  if (!Parse(str, ast)) {
    sl_log_error(LOG_TAG,
                 "Could not parse UAM mapping file %s.",
                 uam_file_name.c_str());
    return false;
  }

  // Perform the reduction
  //ast::reducer reducer_instance;

  //std::cout << "Before reduction " << ast;
  //ast = reducer_instance(ast);
  //std::cout << "After reduction " << ast;
  return update_dependencies(ast);
}

bool MapperEngine::add_expression(const std::string expression)
{
  ast::ast_tree ast;
  if (Parse(expression, ast)) {
    return update_dependencies(ast);
  } else {
    sl_log_error(LOG_TAG, "Invalid expression: ");
    sl_log_error(LOG_TAG, "-------------------------------");
    sl_log_error(LOG_TAG, "%s", expression.c_str());
    sl_log_error(LOG_TAG, "-------------------------------");
    return false;
  }
}

bool MapperEngine::update_dependencies(const ast::ast_tree &ast)
{
  // First check that the AST has no unknown functions.
  if (has_unknown_functions_in_tree(ast)) {
    sl_log_error(LOG_TAG,
                 "Unknown function name found, "
                 "dependencies will not be updated.");
    return false;
  }

  for (auto ast_root: ast) {
    if (ast_root.type() == typeid(ast::scope)) {
      const auto &scope = boost::get<ast::scope>(ast_root);

      //Go though all assignments in the ast, like r'xxx = yyy
      for (const auto &__assignment: scope.assignments) {
        auto assignment = std::make_shared<ast::assignment>(__assignment);
        auto scope_settings
          = std::make_shared<ast::scope_settings_t>(scope.settings);
        // Use the dependency evaluator to get a list of attributes which this
        // assignment depends on.
        ast::dep_eval dep_eval;
        auto rhs_dep = dep_eval(assignment->rhs);
        std::set<ast::attribute_dependency_t> dependencies;
        dependencies.insert(rhs_dep.begin(), rhs_dep.end());

        //if there is no right hand side dependencies we treat this as an error
        if (dependencies.size() == 0) {
          std::stringstream ss;
          ss << "Constant assignment in " << *assignment;
          sl_log_error(LOG_TAG, ss.str().c_str());
          return false;
        }

        // The left hand side of the assignment is an attribute
        // If there is more than one element in the path this assignment
        // depends on the existence of the parent attribute
        // we use the path evaluator to get left hand side dependencies.
        if (assignment->lhs.attribute_path.size() > 1) {
          ast::dep_eval_path path_eval(assignment->lhs.value_type);
          auto deps = path_eval(*(assignment->lhs.attribute_path.end() - 2));
          dependencies.insert(deps.begin(), deps.end());
        }

        for (const auto &a: dependencies) {
          // for each dependency make a reference back to the assignment
          // such that quickly know which assignment should be re-evaluated
          // then this dependency changes
          relations.insert(std::make_pair(a, assignment));
          assignment_settings[assignment] = scope_settings;

          // Register the a callback on the dependency
          if ((a.second == 'r') || (a.second == 'e')) {
            attribute_store_register_callback_by_type_and_state(
              &on_reported_attribute_update,
              a.first,
              REPORTED_ATTRIBUTE);
          } else if (a.second == 'd') {
            attribute_store_register_callback_by_type_and_state(
              &on_desired_attribute_update,
              a.first,
              DESIRED_ATTRIBUTE);
          }
        }
      }
    }
  }

  sl_log_debug(LOG_TAG, "%i relation(s) detected.", relations.size());
  // TODO Fix cyclic dependency tracker to not count
  // r'a.b[0].c = r'xx
  // r'a.b[1].c = r'xx
  // as a cyclic dependency
  //return false == ast::check_cyclic_dependencies(ast,relations);
  return true;
}

bool MapperEngine::has_unknown_functions_in_tree(const ast::ast_tree &ast)
{
  ast::built_in_function_check function_check;
  return function_check(ast);
}

void MapperEngine::set_common_parent_type(attribute_store_type_t t)
{
  sl_log_debug(LOG_TAG,
               "Mapper engine configured with Common Parent type %d",
               t);
  this->common_parent_type = t;
}

//Get the singletron
MapperEngine &MapperEngine::get_instance()
{
  static MapperEngine instance;  // Guaranteed to be destroyed.
                                 // Instantiated on first use.
  return instance;
}

attribute_store::attribute MapperEngine::get_destination_for_attribute(
  const attribute_store::attribute &endpoint,
  const ast::attribute &attribute,
  bool create_if_missing) const
{
  ast::attribute_path_eval path_evaluator(endpoint);
  attribute_store::attribute dest = path_evaluator(attribute.attribute_path);

  if (path_evaluator.all_elements_parsed()) {
    return dest;
  } else if (path_evaluator.last_token_failed() && create_if_missing) {
    // if the path is a.b.c, we should only create c if
    // a and b can be resolved. c should be created under b
    sl_log_debug(
      LOG_TAG,
      "Creating attribute with type 0x%08x under Attribute ID %d (type=0x%08x)",
      path_evaluator.last_fail_type(),
      dest,
      dest.type());
    return dest.add_node(path_evaluator.last_fail_type());
  } else {
    return ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE;
  }
}

void MapperEngine::on_attribute_updated(
  attribute_store_node_t node,
  attribute_store_node_value_state_t state,
  attribute_store_change_t change)
{
  auto original_node = attribute_store::attribute(node);
  ast::value_type_t updated_value_type;

  if ((change == ATTRIBUTE_CREATED) || (change == ATTRIBUTE_DELETED)) {
    updated_value_type = 'e';
  } else if (state == DESIRED_ATTRIBUTE) {
    updated_value_type = 'd';
  } else if (state == REPORTED_ATTRIBUTE) {
    updated_value_type = 'r';
  } else {
    sl_log_error(LOG_TAG,
                 "Invalid Attribute Store Update state "
                 "(neither Desired nor Reported)");
    return;
  }

  ast::attribute_dependency_t dep
    = std::make_pair(original_node.type(), updated_value_type);

  // find all relations that depend on this type
  auto [first, last] = relations.equal_range(dep);
  for (auto r = first; r != last; r++) {
    const ast::assignment &assignment     = *(r->second);
    const ast::scope_settings_t &settings = *(assignment_settings[r->second]);

    // Check if the scope has a endpoint type override, then locate the parent endpoint.
    attribute_store_type_t scope_common_parent_type
      = get_common_parent_type_scope_configuration(this->common_parent_type,
                                                   settings);
    attribute_store::attribute endpoint
      = original_node.first_parent(scope_common_parent_type);
    if (!endpoint.is_valid()) {
      // Log warning only if the node does not exist. is_valid() return false
      // if the node is marked for deletion.
      if (endpoint == ATTRIBUTE_STORE_INVALID_NODE) {
        sl_log_warning(LOG_TAG,
                       "Unable to locate the parent node while "
                       "evaluating relations after node %d udpate. Verify that "
                       "the Common Parent Type (%d) is set correctly",
                       original_node,
                       scope_common_parent_type);
      }
      continue;
    }

    // attempt evaluation of the right hand side of the assignment
    ast::eval<result_type_t> evaluator(endpoint);
    auto value = evaluator(assignment.rhs);

    if (value) {
      // Verify if we want to create an attribute.
      const bool create_if_missing
        = should_create_attributes(assignment.lhs.value_type,
                                   settings,
                                   value.value());

      // Find the destination, start from the Endpoint node.
      attribute_store::attribute destination_node
        = get_destination_for_attribute(endpoint,
                                        assignment.lhs,
                                        create_if_missing);
      if (!destination_node.is_valid()) {
        continue;
      }

      bool attribute_reactions_paused_by_engine = false;
      if ((false == is_chain_reaction_enabled(settings))
          && (false
              == attribute_mapper_is_attribute_reactions_paused(
                destination_node))) {
        attribute_reactions_paused_by_engine = true;
        attribute_mapper_pause_reactions_to_attribute_updates(destination_node);
      }

#ifndef NDEBUG
      // Debug build will print the matched expressions
      std::stringstream ss;
      ss << "Match expression: " << assignment.lhs
         << " triggered by Attribute ID " << std::dec << original_node << " ("
         << attribute_store_get_type_name(original_node.type()) << ")"
         << " affecting Attribute ID " << std::dec << destination_node
         << " - Result value: " << value.value();
      sl_log_debug(LOG_TAG, ss.str().c_str());
#endif

      try {
        if (assignment.lhs.value_type == 'r') {
          if (change != ATTRIBUTE_DELETED) {
            if (is_clear_desired_value_enabled(settings)) {
              destination_node.clear_desired();
            }
            attribute_store_set_reported_number(destination_node,
                                                value.value());
          }
        } else if (assignment.lhs.value_type == 'd') {
          attribute_store_set_desired_number(destination_node, value.value());
        } else if (assignment.lhs.value_type == 'e') {
          // Existences don't set values, they create attributes
          if (value.value() == 0) {
            sl_log_debug(LOG_TAG, "Deleting Attribute ID %d", destination_node);
            destination_node.delete_node();
          }
          // Creations for the exitence type assignment are handled by
          // should_create_attributes / create_if_missing above.
        }
      } catch (std::invalid_argument const &) {
        sl_log_error(LOG_TAG,
                     "Failed to modify the value of Attribute Store node %d",
                     destination_node);
      }

      if (true == attribute_reactions_paused_by_engine) {
        attribute_mapper_resume_reactions_to_attribute_updates(
          destination_node);
      }
    }
  }
}