#pragma once

#include "commons/utility/utility.h"

struct bin_tree_node {
  var_bitset v;
  bin_tree_node *left;
  bin_tree_node *right;

  bin_tree_node(var_bitset &v1) : v(v1) {}
};

class btree {
  void destroy_tree(bin_tree_node *node);

public:
  bin_tree_node *root;
  ~btree() { destroy_tree(root); }
};
