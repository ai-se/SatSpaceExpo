#include "binaryTree.h"
#include "alglib/dataanalysis.h"
#include "alglib/stdafx.h"
#include <algorithm>
#include <exception>

void btree::destroy_tree(bin_tree_node *node) {
  if (node != NULL) {
    destroy_tree(node->left);
    destroy_tree(node->right);
    delete node;
  }
}

int get_sub_depth(bin_tree_node *node) {
  int ld = 0;
  int rd = 0;
  if (node->left)
    ld = get_sub_depth(node->left);
  if (node->right)
    rd = get_sub_depth(node->right);

  return std::max(ld, rd) + 1;
}

void visting_util(TRA_T order, bin_tree_node *root,
                  std::function<void(bin_tree_node *)> visit) {
  if (root == NULL)
    return;

  if (order == TRA_T_PRE_ORDER)
    visit(root);

  visting_util(order, root->left, visit);

  if (order == TRA_T_IN_ORDER)
    visit(root);

  visting_util(order, root->right, visit);

  if (order == TRA_T_POST_ORDER)
    visit(root);
}

int btree::get_depth() { return get_sub_depth(root); }

void btree::traverse(TRA_T order, std::function<void(bin_tree_node *)> visit) {
  visting_util(order, root, visit);
}

bin_tree_node *btree::find_share_parent(std::vector<size_t> delta_idx) {
  // figure out hash for idx. memorized
  // https://stackoverflow.com/questions/20511347/a-good-hash-function-for-a-vector
  std::size_t hash = delta_idx.size();
  for (size_t i : delta_idx) {
    hash ^= i + 0x9e3779b9 + (hash << 6) + (hash >> 2);
  }
  if (parent_memo.count(hash) > 0)
    return parent_memo[hash];

  bin_tree_node *cursor = root;
  size_t i = 1;
  try {
    while (i++ > 0) {
      char o = delta_at[&deltas[delta_idx[0]]]->path.at(i);
      for (size_t x : delta_idx)
        if (delta_at[&deltas[delta_idx[x]]]->path.at(i) != o)
          throw;
      if (o == '0')
        cursor = cursor->left;
      else
        cursor = cursor->right;
    } // end repeat
  } catch (...) {
  }

  parent_memo.insert(std::make_pair(hash, cursor));
  return cursor;
}

bin_tree_node *create_sub_guide_tree(std::set<var_bitset *> delta_ps,
                                     size_t delta_len) {
  bin_tree_node *subroot = new bin_tree_node;
  subroot->delta_ps = std::move(delta_ps);
  // TODO set leaf condition here!
  if (subroot->delta_ps.size() < 10)
    return subroot;

  // run alglib k-means++ to separte the deltas into two groups
  // TODO change to sway (WHICH) right here
  alglib::clusterizerstate s; // alglib::ahcreport rep;
  alglib::kmeansreport rep;
  alglib::ae_int_t disttype = 2;
  alglib::real_2d_array xy;

  xy.setlength(subroot->delta_ps.size(), delta_len);

  size_t di = 0;
  for (auto d : subroot->delta_ps) {
    for (size_t j = 0; j < d->size(); j++)
      xy(di, j) = d->test(j) ? 1 : 0;
    di++;
  }

  alglib::clusterizercreate(s);
  alglib::clusterizersetpoints(s, xy, disttype);
  alglib::clusterizerrunkmeans(s, 2, rep);
  // END of k-menas++. result stored in rep.cidx

  std::set<var_bitset *> ldelta_ps, rdelta_ps;
  di = 0;
  for (auto d : subroot->delta_ps) {
    if (rep.cidx[di++] == 1)
      ldelta_ps.insert(d);
    else
      rdelta_ps.insert(d);
  }

  subroot->left = create_sub_guide_tree(ldelta_ps, delta_len);
  subroot->right = create_sub_guide_tree(rdelta_ps, delta_len);

  subroot->left->parent = subroot;
  subroot->right->parent = subroot;

  subroot->left->path = subroot->right->path = subroot->path;
  subroot->left->path += '0';
  subroot->right->path += '1';
  return subroot;
}

btree::btree(vbitset_vec_t &samples) {
  // find out all deltas
  std::map<var_bitset, int> delta_cnt;
  for (size_t i = 0; i < samples.size(); i++)
    for (size_t j = i + 1; j < samples.size(); j++) {
      auto d = samples[i] ^ samples[j];
      if (delta_cnt.count(d))
        delta_cnt[d] += 1;
      else
        delta_cnt.insert(std::make_pair(d, 1));
    }

  int total_deltas = 0;
  for (auto &pair : delta_cnt) {
    deltas.push_back(pair.first);
    total_deltas += pair.second;
  }

  accmu_inst_weight.push_back(0);
  for (auto &pair : delta_cnt)
    accmu_inst_weight.push_back(accmu_inst_weight.back() +
                                pair.second /
                                    static_cast<double>(total_deltas));

  // build the tree recursively
  std::set<var_bitset *> tmp_ds;
  for (size_t i = 0; i < deltas.size(); i++)
    tmp_ds.insert(&deltas[i]);
  size_t vars_num = deltas[0].size();
  root = create_sub_guide_tree(tmp_ds, vars_num);

  // record_info_after_built
  root->path = "r";
  // traverse(TRA_T_PRE_ORDER,
  //          [&](bin_tree_node *node) { all_node_ps.push_back(node); });

  traverse(TRA_T_PRE_ORDER, [&](bin_tree_node *node) {
    if (!node->left && !node->right) {
      for (var_bitset *dp : node->delta_ps) {
        delta_at.insert(std::make_pair(dp, node));
      }
    }
  });

  // marking the union and intersection var_bitste for each tree node
  traverse(TRA_T_POST_ORDER, [&](bin_tree_node *node) {
    node->union_delta.resize(vars_num, false);
    node->intersection_delta.resize(vars_num, true);
    for (auto d : node->delta_ps) {
      node->union_delta |= *d;
      node->intersection_delta &= *d;
    }
  });
}

std::vector<size_t> btree::rnd_pick_idx_based_on_probability(size_t cnt) {
  std::set<size_t> idx;
  while (idx.size() < cnt) {
    double r = (double)rand() / RAND_MAX;
    size_t index = std::lower_bound(accmu_inst_weight.begin(),
                                    accmu_inst_weight.end(), r) -
                   accmu_inst_weight.begin() - 1;
    idx.insert(index);
  }

  return std::vector<size_t>(idx.begin(), idx.end());
}