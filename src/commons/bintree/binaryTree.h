#pragma once

#include "commons/utility/utility.h"
#include <functional>
#include <utility>
#include <vector>

enum TRA_T { TRA_T_PRE_ORDER, TRA_T_IN_ORDER, TRA_T_POST_ORDER };

struct bin_tree_node {
  // metadata
  std::set<var_bitset> deltas;
  var_bitset union_delta;
  var_bitset intersection_delta;
  cpset_t should_verify;
  // end of metadata

  bin_tree_node *left = NULL;
  bin_tree_node *right = NULL;
  bin_tree_node *parent = NULL;
};

int get_sub_depth(bin_tree_node *node);

class btree {
  void destroy_tree(bin_tree_node *node);

public:
  bin_tree_node *root;
  bool node_union_inter_delta_set = false;

  int get_depth();
  void traverse(TRA_T order, std::function<void(bin_tree_node *)> visit);
  ~btree() { destroy_tree(root); }
};
