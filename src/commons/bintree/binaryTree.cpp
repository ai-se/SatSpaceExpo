#include "binaryTree.h"
#include <algorithm>

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
};
