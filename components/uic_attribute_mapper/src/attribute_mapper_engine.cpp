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
#include "attribute_mapper_ast.hpp"
#include "attribute_mapper_ast_print.hpp"
#include "attribute_mapper_ast_eval.hpp"
#include "attribute_mapper_ast_dep_eval.hpp"
#include "attribute_mapper_ast_path_match_eval.hpp"
#include "attribute_mapper_cyclic_check.hpp"
#include "attribute_mapper_ast_reducer.hpp"
#include "attribute_mapper_built_in_functions.hpp"
#include "attribute_mapper_scope_settings.hpp"

#include "sl_log.h"
#include <map>
#include <fstream>
#include <tuple>
#include <assert.h>
#include <boost/filesystem.hpp>

constexpr const char LOG_TAG[] = "attribute_mapper";

using namespace attribute_store;

using assignment_set_t = std::set<std::shared_ptr<ast::assignment>>;

////////////////////////////////////////////////////////////////////////////////
// MapperEngine Class functions
////////////////////////////////////////////////////////////////////////////////
bool MapperEngine::assignment_matches_destination(
  std::shared_ptr<ast::assignment> assignment, attribute destination)
{
  if (!destination.is_valid()) {
    return false;
  }

  const ast::scope_settings_t &settings = *(assignment_settings[assignment]);
  attribute_store_type_t scope_common_parent_type
    = get_common_parent_type_scope_configuration(this->common_parent_type,
                                                 settings);

  // Here we take the destination, start from the end of the LHS and climb up
  // the attribute store tree to see if everything matches
  ast::path_match_eval path_match(destination, scope_common_parent_type);
  return path_match(assignment->lhs.attribute_path);
}

attribute MapperEngine::get_assigment_potential_destination(
  std::shared_ptr<ast::assignment> assignment, attribute original_node)
{
  attribute destination_node;
  const ast::scope_settings_t &settings = *(assignment_settings[assignment]);

  // Check if the scope has a endpoint type override, then locate the parent endpoint.
  attribute_store_type_t scope_common_parent_type
    = get_common_parent_type_scope_configuration(this->common_parent_type,
                                                 settings);
  attribute common_parent
    = original_node.first_parent(scope_common_parent_type);
  if (!common_parent.is_valid()) {
    return destination_node;
  }

  // Find the destination, start from the Common parent type node.
  destination_node
    = get_destination_for_attribute(common_parent, assignment->lhs, false);
  return destination_node;
}

equivalent_assignments_t MapperEngine::get_equivalent_assignments(
  std::shared_ptr<ast::assignment> initial_assigment)
{
  equivalent_assignments_t equivalent_assignments;

  if (assignment_properties.count(initial_assigment) == 0) {
    sl_log_error(LOG_TAG, "No equivalent assignment found");
    return equivalent_assignments;
  }
  auto initial_props = assignment_properties[initial_assigment];

  // Find all assignments that have identical value_type / attribute type.
  for (const auto &[assignment, properties]: assignment_properties) {
    if ((std::get<1>(properties) == std::get<1>(initial_props))
        && (std::get<2>(properties) == std::get<2>(initial_props))) {
      equivalent_assignments.insert(
        {std::get<0>(assignment_properties[assignment]), assignment});
    }
  }

  //sl_log_debug(LOG_TAG,
  //             "Number of equivalent assignments for %c'%s: %d",
  //             std::get<1>(initial_props),
  //             attribute_store_get_type_name(std::get<2>(initial_props)),
  //             equivalent_assignments.size());
  return equivalent_assignments;
}

void MapperEngine::reset()
{
  relations.clear();
  assignment_settings.clear();
  assignment_properties.clear();
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
    if (ast_root.type() != typeid(ast::scope)) {
      continue;
    }
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

      // Now make sure to capture assignment settings/properties in our cache.
      assignment_settings[assignment] = scope_settings;
      ast::dep_eval_path lhs_last_type_eval(assignment->lhs.value_type);
      auto last_attribute_deps
        = lhs_last_type_eval(assignment->lhs.attribute_path.back());
      if (last_attribute_deps.size() != 1) {
        sl_log_error(LOG_TAG,
                     "Last attribute dependencies unexpected size: %lu",
                     last_attribute_deps.size());
        std::stringstream ss;
        ss << "Assignment props for: " << assignment->lhs;
        sl_log_error(LOG_TAG, ss.str().c_str());
        return false;
      }

      attribute_store_type_t assigned_type = last_attribute_deps[0].first;
      assignment_properties[assignment]
        = std::make_tuple(scope.priority,
                          assignment->lhs.value_type,
                          assigned_type);

      for (const auto &a: dependencies) {
        // for each dependency make a reference back to the assignment
        // such that quickly know which assignment should be re-evaluated
        // then this dependency changes
        relations.insert(std::make_pair(a, assignment));

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

attribute MapperEngine::get_destination_for_attribute(
  const attribute &endpoint,
  const ast::attribute &lhs_attribute,
  bool create_if_missing) const
{
  ast::attribute_path_eval path_evaluator(endpoint);
  attribute dest = path_evaluator(lhs_attribute.attribute_path);

  if (path_evaluator.all_elements_parsed()) {
    return dest;
  } else if (path_evaluator.last_token_failed() && create_if_missing) {
    // if the path is a.b.c, we should only create c if
    // a and b can be resolved. c should be created under b
    sl_log_debug(LOG_TAG,
                 "Creating attribute with type 0x%08x under Attribute ID %d "
                 "(type=0x%08x)",
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
  auto original_node = attribute(node);
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

  if (change == ATTRIBUTE_DELETED) {
    // Attribute deletion sometimes affect reported values. Evaluate
    // a deletion as a reported attribute update too.
    this->on_attribute_updated(node, REPORTED_ATTRIBUTE, ATTRIBUTE_UPDATED);
  }

  ast::attribute_dependency_t dep
    = std::make_pair(original_node.type(), updated_value_type);

  // find all relations that depend on this type, store them in a set.
  auto [first, last] = relations.equal_range(dep);

  // Keep a container to avoid running the same
  // equivalent_assignments / destination node several times in a row
  assignments_to_run_t assignments_to_run;
  for (auto r = first; r != last; r++) {
    // Check if the potential destination already exists.
    attribute initial_destination
      = get_assigment_potential_destination(r->second, original_node);

    // Find concurent assignments that may have priority.
    equivalent_assignments_t assignments_to_check
      = get_equivalent_assignments(r->second);

    // Add it to the list. if destination is the same, it will be filtered out.
    if (initial_destination.is_valid()) {
      assignments_to_run[initial_destination] = assignments_to_check;
    } else {
      this->run_assignments(assignments_to_check,
                            initial_destination,
                            original_node);
    }
  }

  // Now run everything, the map should have filtered duplicates.:
  for (auto &[destination, assignments_to_check]: assignments_to_run) {
    this->run_assignments(assignments_to_check, destination, original_node);
  }
}

void MapperEngine::run_assignments(
  equivalent_assignments_t assignments_to_check,
  attribute assigment_destination,
  attribute original_node)
{
  const bool run_all_assignments = !assigment_destination.is_valid();

  // If the destination is unknown, we run all assignment in ascending priority,
  // the last one having an effect will prevail (highest priority)
  if (run_all_assignments) {
    sl_log_debug(LOG_TAG,
                 "Running all %d equivalent assignments in ascending priority "
                 "after Attribute ID %d update due to missing destination",
                 assignments_to_check.size(),
                 original_node);
    for (auto it = assignments_to_check.rbegin();
         it != assignments_to_check.rend();
         ++it) {
      this->run_assignment(it->second, assigment_destination, original_node);
    }
  } else {
    // Anouncement
    sl_log_debug(LOG_TAG,
                 "Checking assignments for Destination ID "
                 "%d (%s), Original Node %d (%s). (%d candidate(s))",
                 assigment_destination,
                 attribute_store_get_type_name(assigment_destination.type()),
                 original_node,
                 attribute_store_get_type_name(original_node.type()),
                 assignments_to_check.size());

    // Stop whenever the highest priority assignment yielded a value.
    for (auto &[priority, assignment]: assignments_to_check) {
      bool executed = this->run_assignment(assignment,
                                           assigment_destination,
                                           original_node);

      if (executed) {
        sl_log_debug(LOG_TAG,
                     "Assigment with priority %d executed successfully.",
                     priority);
        return;
      }
    }
    sl_log_debug(LOG_TAG, "No assignment yielded a value.");
  }
}

bool MapperEngine::run_assignment(std::shared_ptr<ast::assignment> assignment,
                                  attribute destination,
                                  attribute original_node)
{
  // If we have a defined destination, we want the assignment LHS path to match it.
  if (destination.is_valid()
      && assignment_matches_destination(assignment, destination) == false) {
    return false;
  }

  const ast::scope_settings_t &settings = *(assignment_settings[assignment]);

  // Check if the scope has a Common Parent type override, then locate the common parent.
  attribute_store_type_t scope_common_parent_type
    = get_common_parent_type_scope_configuration(this->common_parent_type,
                                                 settings);

  // In a few places we search the common parent by going up the tree, but it
  // will fail if there are several "common parent types" along this tree.
  // Ideally we should use a navigation like for the ast_path_eval.
  attribute common_parent;
  if (destination.is_valid()) {
    common_parent = destination.first_parent(scope_common_parent_type);
  } else {
    common_parent = original_node.first_parent(scope_common_parent_type);
  }
  if (!common_parent.is_valid()) {
    // Log warning only if the node does not exist. is_valid() return false
    // if the node is marked for deletion.
    if (common_parent == ATTRIBUTE_STORE_INVALID_NODE) {
      sl_log_debug(LOG_TAG,
                   "Unable to locate the parent node while evaluating "
                   "relations after node %d udpate. Double check that the "
                   "Common Parent Type (%d) is correct. Skipping assignment",
                   original_node,
                   scope_common_parent_type);
    }
    return false;
  }

  // attempt evaluation of the right hand side of the assignment
  ast::eval<result_type_t> evaluator(common_parent);
  auto value = evaluator(assignment->rhs);

  if (!value) {
    return false;
  }

  // Verify if we want to create an attribute.
  const bool create_if_missing
    = should_create_attributes(assignment->lhs.value_type,
                               settings,
                               value.value());

  // Find a new destination, in case we need to create the attribute if missing
  destination = get_destination_for_attribute(common_parent,
                                              assignment->lhs,
                                              create_if_missing);
  if (!destination.is_valid()) {
    return false;
  }

  bool attribute_reactions_paused_by_engine = false;
  if ((false == is_chain_reaction_enabled(settings))
      && (false
          == attribute_mapper_is_attribute_reactions_paused(destination))) {
    attribute_reactions_paused_by_engine = true;
    attribute_mapper_pause_reactions_to_attribute_updates(destination);
  }

#ifndef NDEBUG
  // Debug build will print the matched expressions
  std::stringstream ss;
  ss << "Match expression: " << assignment->lhs << " triggered by Attribute ID "
     << std::dec << original_node << " ("
     << attribute_store_get_type_name(original_node.type()) << ")"
     << " affecting Attribute ID " << std::dec << destination
     << " - Result value: " << value.value();
  sl_log_debug(LOG_TAG, ss.str().c_str());
#endif

  try {
    if (assignment->lhs.value_type == 'r') {
      if (is_clear_desired_value_enabled(settings)) {
        destination.clear_desired();
      }
      attribute_store_set_reported_number(destination, value.value());

    } else if (assignment->lhs.value_type == 'd') {
      attribute_store_set_desired_number(destination, value.value());
    } else if (assignment->lhs.value_type == 'e') {
      // Existences don't set values, they create attributes
      if (value.value() == 0) {
        sl_log_debug(LOG_TAG, "Deleting Attribute ID %d", destination);
        destination.delete_node();
      }
      // Creations for the exitence type assignment are handled by
      // should_create_attributes / create_if_missing above.
    }
  } catch (std::invalid_argument const &) {
    sl_log_error(LOG_TAG,
                 "Failed to modify the value of Attribute Store node %d",
                 destination);
  }

  if (true == attribute_reactions_paused_by_engine) {
    attribute_mapper_resume_reactions_to_attribute_updates(destination);
  }

  // We executed it, return true;
  return true;
}
