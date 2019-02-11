#include "UDG.h"
#include "commons/clause/clause.h"

#include <iostream>

template <class T> T UDG<T>::nextNewCompoment(std::map<T, bool> &visited) {
  for (auto &i : nodes)
    if (!visited[i])
      return i;
  return 0;
}

template <class T> int UDG<T>::get_edge_num() {
  int num = 0;
  for (auto &n : nodes)
    if (edges.find(n) != edges.end())
      num += edges[n].size();
  return num;
}

template <class T> void UDG<T>::add_node(T i) { nodes.insert(i); }

template <class T> void UDG<T>::add_edge(T i, T j) {
  edges[i].insert(j);
  edges[j].insert(i);
}

template <class T>
void UDG<T>::DFSUtil(T v, std::map<T, bool> &visited, std::set<T> &visiting) {
  csize++;
  visited[v] = true;
  visiting.insert(v);
  for (auto &i : edges[v])
    if (!visited[i])
      DFSUtil(i, visited, visiting);
}

template <class T> int UDG<T>::count_component_nums() {
  return de_components().size();
}

template <class T> std::vector<std::set<T>> UDG<T>::de_components() {
  // NOTE Here we DONT COUNT SINGLE NODE COMPONENTS
  std::map<T, bool> visited;
  std::vector<std::set<T>> component_info;
  for (auto &i : nodes)
    visited[i] = false;

  int components = 0;
  T s;
  while ((s = nextNewCompoment(visited))) {
    csize = 0;
    std::set<T> visiting;
    DFSUtil(s, visited, visiting);
    if (csize >= 2) {
      components++;
      component_info.push_back(visiting);
    }
  }
  return component_info;
}

template class UDG<int>;
template class UDG<clause_t *>;
