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
#include "attribute_mapper_ast_eval.hpp"
#include "attribute_mapper_ast_dep_eval.hpp"

#include "sl_log.h"
#include <map>
#include <fstream>
#include <assert.h>
#include <boost/filesystem.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/edge_list.hpp>
#include <boost/graph/depth_first_search.hpp>

#include "attribute_mapper_ast_print.hpp"

static const char *LOG_TAG = "attribute_mapper";

/* Visitor class for detecting cyclic dependencies
 * https://www.boost.org/doc/libs/1_55_0/libs/graph/doc/file_dependency_example.html#sec:cycles
 */
template<class EdgeType> struct cycle_detector : public boost::dfs_visitor<> {
  cycle_detector(bool &has_cycle) : _has_cycle(has_cycle) {}

  template<class Edge, class Graph> void back_edge(Edge e, Graph &g)
  {
    _has_cycle        = true;
    _last_cyclic_edge = std::make_pair(e.m_source, e.m_target);
  }
  EdgeType _last_cyclic_edge;
  bool &_has_cycle;
};

/**
 * @brief Helper for building a index table of a set of values
 *
 * Each element in the set will be assigned an index.
 *
 * @tparam T Type of elements
 */
template<typename T> class IndexTable
{
  public:
  /**
   * @brief Return index of an element
   *
   * If the element is not in the table, a new index is created.
   *
   * @param v element
   * @return size_t
   */
  size_t operator()(const T &v)
  {
    if (index_table.find(v) == index_table.end()) {
      index_table.insert(std::make_pair(v, index_table.size()));
      rev_index_table.push_back(v);
    }
    return index_table[v];
  }

  /**
   * @brief Lookup an element by its index.
   *
   * @param idx
   * @return const T&
   */
  const T &operator()(size_t idx)
  {
    return rev_index_table[idx];
  }

  private:
  std::map<T, size_t> index_table;
  std::vector<T> rev_index_table;
};

static bool locate_end_point(attribute_store::attribute &ep_node,
                             const attribute_store_type_t &ep_type)
{
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
  while ((ep_node.is_valid()) && (ep_node.type() != ep_type)) {
    ep_node = ep_node.parent();
  }
  return ep_node.is_valid();
}

void MapperEngine::reset()
{
  ast.clear();
  relations.clear();
}

bool MapperEngine::load_path(const std::string &uam_path)
{
  using namespace boost::filesystem;

  sl_log_debug(LOG_TAG, "Initalising attribute mapper engine");

  std::vector<std::string> uam_files;

  if (!is_directory(uam_path)) {
    sl_log_error(LOG_TAG, "UAM directory %s not found.", uam_path.data());
    return false;
  }
  for (auto &p: directory_iterator(uam_path)) {
    //      sl_log_info(LOG_TAG, "file %s",p.path().extension().c_str());
    if (is_regular_file(p.path()) && p.path().extension() == ".uam") {
      uam_files.push_back(p.path().string());
    }
  }

  //Read all mapping files
  for (auto uam_file_name: uam_files) {
    if (!load_file(uam_file_name)) {
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

  //sl_log_debug(LOG_TAG, "Initalising attribute mapper engine");

  if (!is_regular_file(uam_file_name)) {
    sl_log_error(LOG_TAG, "specificed UAM file is invalid");
    return false;
  }

  //Read mapping file
  sl_log_info(LOG_TAG, "Loading mapping file %s", uam_file_name.c_str());

  std::ifstream uam_file(uam_file_name);
  if (!uam_file) {
    sl_log_error(LOG_TAG,
                 "Could not open mapping file %s.",
                 uam_file_name.c_str());
    return false;
  }
  std::string str((std::istreambuf_iterator<char>(uam_file)),
                  std::istreambuf_iterator<char>());

  if (!Parse(str, ast)) {
    sl_log_error(LOG_TAG,
                 "Could not parse mapping file %s.",
                 uam_file_name.c_str());
    return false;
  }

  return update_dependencies();
}

bool MapperEngine::add_expression(const std::string expression)
{
  if (Parse(expression, ast)) {
    return update_dependencies();
  } else {
    sl_log_error(LOG_TAG, "Invalid expression: ");
    sl_log_error(LOG_TAG, "-------------------------------");
    sl_log_error(LOG_TAG, "%s", expression.c_str());
    sl_log_error(LOG_TAG, "-------------------------------");
    return false;
  }
}

bool MapperEngine::check_cyclic_dependencies()
{
  using namespace ast;
  using namespace boost;
  // We use the boost graph library to build a DAG of the relations
  // create a typedef for the Graph type
  typedef adjacency_list<vecS, vecS, bidirectionalS> Graph;
  typedef std::pair<size_t, size_t> Edge;

  // The boost adjacency_list graph uses model where each node in graph is
  // identified by an index and the edges are defined as connections between the
  // nodes(referenced by an index). This is why we need to translate our
  // attribute_dependency_t into an index in a table, this is again achieved by
  // putting all the attribute_dependency_t into a map where the value is the
  // index.
  std::vector<Edge> edges;
  size_t vertex_count = 0;

  // We need to go though all the realtions to build the graph
  // ie, the expressions:
  //   r'1111 = r'2222 + 5*r'3333
  //   r'3333 = if( r'4444 == 3 ) 2 d'1111
  // should produce the graph:
  //   r'1111 <- r'2222
  //   r'1111 <- r'3333
  //   r'3333 <- r'4444
  //   r'3333 <- d'1111
  // In order accommodate boost graph algorithm we translate this into
  //  0 <- 1
  //  0 <- 2
  //  2 <- 3
  //  2 <- 4
  IndexTable<ast::attribute_dependency_t> index_table;
  for (const auto &r: relations) {
    const ast::assignment &assignment = r.second;
    // Locate the type of the left hand side of the assignment, ie before the equal
    // We can use the dependency evaluatogit r for this.
    ast::dep_eval_path path_eval(ast, assignment.attribute.value_type);
    path_eval(assignment.attribute.attribute);
    const ast::attribute_dependency_t &src
      = *path_eval.get_dependencies().begin();
    const ast::attribute_dependency_t &dst = r.first;

    //Added the index converted edge
    edges.push_back(std::make_pair(index_table(src), index_table(dst)));
  }

  // Construct the Graph
  Graph g(edges.begin(), edges.end(), vertex_count);

  // Use the cycle detector visitor to check if the graph has a cyclic dependency
  // If the visitor encouters a back_edge then there is a cyclic dependency.
  // The visitor uses a refernce to the has_cycle which means that this variable
  // serves as an output.
  bool has_cycle = false;
  cycle_detector<Edge> vis(has_cycle);
  boost::depth_first_search(g, visitor(vis));

  if (has_cycle) {
    sl_log_error(
      LOG_TAG,
      "Cyclic dependency detected from %i to %i. Please check your UAM files.",
      index_table(vis._last_cyclic_edge.first).first,
      index_table(vis._last_cyclic_edge.second).first);
  }
  return vis._has_cycle;
}

bool MapperEngine::update_dependencies()
{
  relations.clear();
  for (auto ast_root: ast) {
    if (ast_root.type() == typeid(ast::scope)) {
      const auto scope = boost::get<ast::scope>(ast_root);

      //Go though all assignments in the ast, like r'xxx = yyy
      for (auto ass: scope.assignments) {
        // Use the dependency evaluator to get a list of attributes which this
        // assignment depends on.
        ast::dep_eval dep_eval(ast);
        dep_eval(ass.rhs);
        const std::set<ast::attribute_dependency_t> &dependencies
          = dep_eval.get_dependencies();

        if (dependencies.size() == 0) {
          ast::dep_eval_path path_eval(ast, ass.attribute.value_type);
          path_eval(ass.attribute.attribute);
          sl_log_error(LOG_TAG,
                       "Assignment to attribute %d is constant",
                       *path_eval.get_dependencies().begin());
          return false;
        }
        for (const auto &a: dependencies) {
          // for each dependency make a reference back to the assignment
          // such that quickly know which assignment should be re-evaluated
          // then this dependency changes
          relations.insert(std::make_pair(a, ass));

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
  return false == check_cyclic_dependencies();
}

//Get the singletron
MapperEngine &MapperEngine::get_instance()
{
  static MapperEngine instance;  // Guaranteed to be destroyed.
                                 // Instantiated on first use.
  return instance;
}

bool MapperEngine::get_destination_for_attribute(
  attribute_store::attribute &dest,
  const ast::attribute &attribute,
  bool create_if_missing) const
{
  ast::attribute_path_eval path_evaluator(dest, ast);
  dest = path_evaluator(attribute.attribute);

  if (path_evaluator.last_token_failed() && create_if_missing) {
    // if the path is a.b.c, we should only create c if
    // a and b can be resolved. c should be created under b
    sl_log_debug(
      LOG_TAG,
      "Creating attribute with type %08x under Attribute ID %d (type=%08x)",
      path_evaluator.last_fail_type(),
      dest,
      dest.type());
    dest = dest.add_node(path_evaluator.last_fail_type());
    return true;
  }

  return path_evaluator.all_elements_parsed();
}

void MapperEngine::on_attribute_updated(
  attribute_store::attribute const &original_node,
  attribute_store_node_value_state_t state,
  attribute_store_change_t change)
{
  attribute_store::attribute node(original_node);
  ast::value_type_t updated_value_type;

  if ((change == ATTRIBUTE_CREATED) || (change == ATTRIBUTE_DELETED)) {
    updated_value_type = 'e';
  } else if (state == DESIRED_ATTRIBUTE) {
    updated_value_type = 'd';
  } else if (state == REPORTED_ATTRIBUTE) {
    updated_value_type = 'r';
  } else {
    sl_log_error(LOG_TAG, "Invalid update state");
    return;
  }

  ast::attribute_dependency_t dep
    = std::make_pair(node.type(), updated_value_type);

  // find all relations that depend on this type
  for (auto r = relations.find(dep); r != relations.end(); r++) {
    const ast::assignment &ass = (*r).second;

    if (!locate_end_point(node, ep_type)) {
      sl_log_error(LOG_TAG, "Unable to locate ep_node");
      return;
    }

    // attempt evaluation of expression in relation
    ast::eval evaluator(node, ast);
    ast::result_type value = evaluator(ass.rhs);

    // Find the destination, start from the Endpoint node.
    attribute_store::attribute destination_node(node);
    if (value
        && get_destination_for_attribute(destination_node,
                                         ass.attribute,
                                         ass.attribute.value_type == 'r')) {
      std::stringstream ss;
      ast::print printer(ss);
      printer(ass);
      sl_log_debug(LOG_TAG, "Match expression %s", ss.str().c_str());
      // Uncomment this to check dependencies
      //attribute_store_log_node(original_node, true);
      //attribute_store_log_node(destination_node, true);

      if (ass.attribute.value_type == 'r') {
        try {
          destination_node.set_reported<int32_t>(value.value());
          destination_node.clear_desired();
        } catch (std::invalid_argument const &e) {
          sl_log_warning(
            LOG_TAG,
            "Failed to modify the reported value of Attribute Store node %d",
            destination_node);
        }
      } else if (ass.attribute.value_type == 'd') {
        try {
          destination_node.set_desired<int32_t>(value.value());
        } catch (std::invalid_argument const &e) {
          sl_log_warning(
            LOG_TAG,
            "Failed to modify the desired value of Attribute Store node %d",
            destination_node);
        }
      } else if (ass.attribute.value_type == 'e') {
        //don't set a value just create the attribute
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
