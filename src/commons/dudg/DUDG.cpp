#include "DUDG.h"
#include "commons/utility/utility.h"
#include <iostream>

/**
 * ignore this function
 */
template <class T> int DUDG<T>::get_edge_num() {
  // std::cout << "WARNING : don't use the get_edge_num @ DUDG " << std::endl;
  return edges_num;
  return -1;
}

template <class T> void DUDG<T>::add_node(T i) {
  nodes.insert(i);
  parent[i] = i; // initially, set parent as new separate component
  rank[i] = 0;
}

template <class T> void DUDG<T>::clear_edges() {
  edges_num = 0;
  for (auto &i : nodes) {
    parent[i] = i;
    rank[i] = 0;
  }
}

template <class T> T DUDG<T>::finds(const T &x) { // with path compression
  if (parent[x] == x) {
    return x;
  } else {
    T result = finds(parent[x]);
    parent[x] = result;
    return result;
  }
}

template <class T> void DUDG<T>::unions(T &x, T &y) { // union-by-rank
  T xRoot = finds(x), yRoot = finds(y);
  if (xRoot == yRoot)
    return;

  if (rank[xRoot] < rank[yRoot]) {
    parent[xRoot] = yRoot;
  } else if (rank[yRoot] < rank[xRoot]) {
    parent[yRoot] = xRoot;
  } else {
    parent[yRoot] = xRoot;
    rank[xRoot] += 1;
  }
}

template <class T> void DUDG<T>::add_edge(T i, T j) {
  edges_num++;
  unions(i, j);
}

template <class T> std::vector<std::set<T>> DUDG<T>::de_components() {
  // for (auto i : nodes)
  //   std::cout << parent[i] << " ";
  // std::cout << std::endl;
  // abort();

  std::map<T, std::set<T>> tmp_map;
  for (T i : nodes) {
    T iRoot = finds(i);
    tmp_map[iRoot].insert(i);
  }

  // reconstruct the result
  std::vector<std::set<T>> res;
  for (auto it = tmp_map.begin(); it != tmp_map.end(); ++it) {
    if (it->second.size() > 1)
      res.push_back(std::move(it->second));
  }
  return res;
}

template <class T> int DUDG<T>::count_component_nums() {
  return de_components().size();
}

template class DUDG<int>;
