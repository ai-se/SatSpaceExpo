#pragma once

#include <map>
#include <set>
#include <vector>

template <class T> class UDG {
  std::set<T> nodes;
  std::map<T, std::set<T>> edges;
  int csize;

  T nextNewCompoment(std::map<T, bool> &visited);
  void DFSUtil(T, std::map<T, bool> &visited, std::set<T> &visiting);

public:
  UDG(){};
  int get_node_num() { return nodes.size(); }
  int get_edge_num();
  std::set<T> get_nodes() { return nodes; }

  void add_node(T i);
  void add_edge(T i, T j);
  void clear_edges() { edges.clear(); }

  std::vector<std::set<T>> de_components();
  int count_component_nums();
};
