#pragma once

#include <map>
#include <set>
#include <vector>

/**
 * Exactly the same API with UDG.
 * With data structure disjoint set inherient.
 */
template <class T> class DUDG {
  std::set<T> nodes;
  int edges_num = 0;

  std::map<T, T> parent;
  std::map<T, int> rank;

  T finds(const T &x);
  void unions(T &x, T &y);

public:
  DUDG(){};
  int get_node_num() { return nodes.size(); }
  int get_edge_num();
  std::set<T> get_nodes() { return nodes; }

  void add_node(T i);
  void add_edge(T i, T j);
  void clear_edges();

  std::vector<std::set<T>> de_components();
  int count_component_nums();
};
