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
#include "attribute_mapper_parser.hpp"
#include "attribute_mapper_ast_print.hpp"
#include "attribute_mapper_ast_eval.hpp"
#include "attribute_mapper_ast_dep_eval.hpp"
#include "attribute_mapper_cyclic_check.hpp"
#include "sl_log.h"
#include <map>
#include <fstream>
#include <assert.h>
#include <boost/filesystem.hpp>

constexpr const char *LOG_TAG = "attribute_mapper";

void MapperEngine::reset()
{
  relations.clear();
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

  //Reduce the ast
  //ast::reducer reducer;
  //reduce(ast);
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
  for (auto ast_root: ast) {
    if (ast_root.type() == typeid(ast::scope)) {
      const auto scope = boost::get<ast::scope>(ast_root);

      //Go though all assignments in the ast, like r'xxx = yyy
      for (const auto &__assignment: scope.assignments) {
        std::shared_ptr<ast::assignment> assignment
          = std::make_shared<ast::assignment>(__assignment);
        // Use the dependency evaluator to get a list of attributes which this
        // assignment depends on.
        ast::dep_eval dep_eval;
        auto rhs_dep = dep_eval(assignment->rhs);
        std::set<ast::attribute_dependency_t> dependencies;
        dependencies.insert(rhs_dep.begin(), rhs_dep.end());

        //if there is no right hand side dependencies we treat this as an error
        if (dependencies.size() == 0) {
          std::stringstream ss;
          ss << "Constant assignment in " << assignment;
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

          // Register the a callback on the dependency
          if ((a.second == 'r') || (a.second == 'e')) {
            attribute_store_register_callback_by_type_and_state(
              MapperEngine::on_attribute_updated_reported,
              a.first,
              REPORTED_ATTRIBUTE);
          } else if (a.second == 'd') {
            attribute_store_register_callback_by_type_and_state(
              MapperEngine::on_attribute_updated_desired,
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
      "Creating attribute with type %08x under Attribute ID %d (type=%08x)",
      path_evaluator.last_fail_type(),
      dest,
      dest.type());
    return dest.add_node(path_evaluator.last_fail_type());
  } else {
    return ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE;
  }
}

void MapperEngine::on_attribute_updated(
  attribute_store::attribute const &original_node,
  attribute_store_node_value_state_t state,
  attribute_store_change_t change)
{
  ast::value_type_t updated_value_type;

  if ((change == ATTRIBUTE_CREATED) || (change == ATTRIBUTE_DELETED)) {
    updated_value_type = 'e';
  } else if (state == DESIRED_ATTRIBUTE) {
    updated_value_type = 'd';
  } else if (state == REPORTED_ATTRIBUTE) {
    updated_value_type = 'r';
  } else {
    sl_log_error(
      LOG_TAG,
      "Invalid Attribute Store Update state (neither Desired nor Reported)");
    return;
  }

  /* Locate the endpoint node
    * FIXME: this is not the optimal solution. It would be better to
    * store the path depth in the relations such that we could just
    * go a number of levels up the the tree to find the common parent.
    *
    * Right releation r'aaa = r'bbb only works if the parent of aaa and bbb is
    * of type ep_type.
    *
    * It would be nice could say the the sematics above just mean that aaa and
    * bbb has A common parrent. Which could be of any type. But this requires
    * us to note that for the expression r'aaa = r'bbb.ccc[1].ddd we note that
    * when ddd changes we need to go 3 up to find the common parent of aaa and ddd
    */
  attribute_store::attribute endpoint = original_node.first_parent(ep_type);

  if (!endpoint.is_valid()) {
    sl_log_error(LOG_TAG,
                 "Unable to locate the parent Endpoint node while evaluating "
                 "relations after node %d udpate",
                 original_node);
    return;
  }

  ast::attribute_dependency_t dep
    = std::make_pair(original_node.type(), updated_value_type);

  // find all relations that depend on this type
  auto [first, last] = relations.equal_range(dep);
  for (auto r = first; r != last; r++) {
    const ast::assignment &assignment = *(r->second);

    // attempt evaluation of the right hand side of the assignment
    ast::eval evaluator(endpoint);
    ast::result_type value = evaluator(assignment.rhs);

    if (value) {
      std::stringstream ss;
      ss << "Match expression: " << assignment << " triggered by attribute "
         << original_node;
      sl_log_debug(LOG_TAG, ss.str().c_str());

      // Find the destination, start from the Endpoint node.
      attribute_store::attribute destination_node
        = get_destination_for_attribute(endpoint,
                                        assignment.lhs,
                                        assignment.lhs.value_type == 'r');
      if (!destination_node.is_valid()) {
        continue;
      }
      // Uncomment this to check dependencies
      //attribute_store_log_node(original_node, true);
      //attribute_store_log_node(destination_node, true);
      try {
        if (assignment.lhs.value_type == 'r') {
          if (change == ATTRIBUTE_DELETED) {
            destination_node.delete_node();
            //TODO: Now that we have deleted a node we should asynchronously
            // re-evaluate the realtions which depend on the type of the
            // original deleted node, because the relations may be forfilled in
            // a another way ie in the expression r'a = r'b | r'c, here if b is
            // deleted c can forfill the relation
          } else {
            destination_node.set_reported<int32_t>(value.value());
            destination_node.clear_desired();
          }
        } else if (assignment.lhs.value_type == 'd') {
          destination_node.set_desired<int32_t>(value.value());
        } else if (assignment.lhs.value_type == 'e') {
          //don't set a value just create the attribute
        }
      } catch (std::invalid_argument const &) {
        sl_log_error(LOG_TAG,
                     "Failed to modify the value of Attribute Store node %d",
                     destination_node);
      }
    }
  }
}

void MapperEngine::on_attribute_updated_reported(
  attribute_store_node_t n, attribute_store_change_t change)
{
  attribute_store::attribute node(n);
  get_instance().on_attribute_updated(node, REPORTED_ATTRIBUTE, change);
}

void MapperEngine::on_attribute_updated_desired(attribute_store_node_t n,
                                                attribute_store_change_t change)
{
  attribute_store::attribute node(n);
  get_instance().on_attribute_updated(node, DESIRED_ATTRIBUTE, change);
}
