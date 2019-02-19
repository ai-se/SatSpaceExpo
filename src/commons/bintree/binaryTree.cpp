#include "binaryTree.h"
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

void btree::record_node_address() {
  traverse(TRA_T_PRE_ORDER,
           [&](bin_tree_node *node) { all_node_ps.push_back(node); });
}

bin_tree_node *btree::find_share_parent(std::vector<size_t> idx) {
  // figure out hash for idx. memorized
  // https://stackoverflow.com/questions/20511347/a-good-hash-function-for-a-vector
  std::size_t hash = idx.size();
  for (size_t i : idx) {
    hash ^= i + 0x9e3779b9 + (hash << 6) + (hash >> 2);
  }
  if (parent_memo.count(hash) > 0)
    return parent_memo[hash];

  bin_tree_node *cursor = root;
  size_t i = 1;
  try {
    while (i++ > 0) {
      char o = all_node_ps[idx[0]]->path.at(i);
      for (size_t x : idx)
        if (all_node_ps[x]->path.at(i) != o)
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