#pragma once

#include "commons/utility/utility.h"
#include <functional>

enum TRA_T { TRA_T_PRE_ORDER, TRA_T_IN_ORDER, TRA_T_POST_ORDER };

struct bin_tree_node {
  var_bitset v;
  bin_tree_node *left = NULL;
  bin_tree_node *right = NULL;

  bin_tree_node(var_bitset &v1) : v(v1) {}
};

int get_sub_depth(bin_tree_node *node);

class btree {
  void destroy_tree(bin_tree_node *node);

public:
  bin_tree_node *root;
  int get_depth();

  void traverse(TRA_T order, std::function<void(bin_tree_node *)> visit);
  ~btree() { destroy_tree(root); }
};
