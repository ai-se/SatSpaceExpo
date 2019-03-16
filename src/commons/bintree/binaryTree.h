#pragma once

#include "commons/utility/utility.h"
#include <functional>
#include <utility>
#include <vector>

enum TRA_T { TRA_T_PRE_ORDER, TRA_T_IN_ORDER, TRA_T_POST_ORDER };

struct bin_tree_node {
  // metadata
  std::set<var_bitset *> delta_ps;
  var_bitset union_delta;
  var_bitset intersection_delta;
  cpset_t should_verify;
  std::string path; // 0-toleeft 1-toright
  // end of metadata

  bin_tree_node *left = NULL;
  bin_tree_node *right = NULL;
  bin_tree_node *parent = NULL;
};

int get_sub_depth(bin_tree_node *node);
bin_tree_node *create_sub_guide_tree(std::set<var_bitset *> delta_ps,
                                     size_t delta_len);

class btree {
  void destroy_tree(bin_tree_node *node);
  std::map<size_t, bin_tree_node *> parent_memo; // caching for delta parents
  std::vector<double> accmu_inst_weight;         // probability to be sampled
  std::map<var_bitset *, bin_tree_node *> delta_at;
  // std::vector<bin_tree_node *> all_node_ps;

public:
  bin_tree_node *root;
  std::vector<var_bitset> deltas;

  btree(vbitset_vec_t &samples);
  int get_depth();
  void traverse(TRA_T order, std::function<void(bin_tree_node *)> visit);
  std::vector<size_t> rnd_pick_idx_based_on_probability(size_t cnt);
  bin_tree_node *find_share_parent(std::vector<size_t> delta_idx);
  ~btree() { destroy_tree(root); }
};
