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
#include "attribute_mapper_cyclic_check.hpp"
#include "sl_log.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/edge_list.hpp>
#include <boost/graph/depth_first_search.hpp>

constexpr const char *LOG_TAG = "attribute_mapper";

/* Visitor class for detecting cyclic dependencies
 * https://www.boost.org/doc/libs/1_55_0/libs/graph/doc/file_dependency_example.html#sec:cycles
 */
template<class EdgeType> struct cycle_detector : public boost::dfs_visitor<> {
  explicit cycle_detector(bool &has_cycle) : _has_cycle(has_cycle) {}

  template<class Edge, class Graph> void back_edge(Edge e, Graph &)
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

bool ast::check_cyclic_dependencies(
  const ast::ast_tree &ast,
  const std::multimap<ast::attribute_dependency_t, ast::assignment> &relations)
{
  using namespace ast;
  using namespace boost;
  // We use the boost graph library to build a DAG of the relations
  // create a typedef for the Graph type
  using Graph = adjacency_list<vecS, vecS, bidirectionalS>;
  using Edge  = std::pair<size_t, size_t>;

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
    ast::dep_eval_path path_eval(assignment.lhs.value_type);
    path_eval(assignment.lhs.attribute_path);
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