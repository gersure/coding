// Copyright 2013 Jike Inc. All Rights Reserved.
// Author: Liqiang Guo(guoliqiang@jike.com)
// I just want to GH to hss~
// Date  : 2013-09-29 17:30:48
// File  : code.cc
// Brief :

/*
Given a binary tree, check whether it is a mirror of itself (ie, symmetric around its center).
For example, this binary tree is symmetric:

    1
   / \
  2   2
 / \ / \
3  4 4  3
But the following is not:
    1
   / \
  2   2
   \   \
   3    3
Note:
Bonus points if you could solve it both recursively and iteratively.

confused what "{1,#,2,3}" means? > read more on how binary tree is serialized on OJ.


OJ's Binary Tree Serialization:
The serialization of a binary tree follows a level order traversal, where '#' signifies a path terminator where no node exists below.

Here's an example:
   1
  / \
 2   3
    /
   4
    \
     5
The above binary tree is serialized as "{1,2,3,#,#,4,#,#,5}".
*/

/*
 * Run Status: Accepted!
 * Program Runtime: 12 milli secs
 * Progress: 28/28 test cases passed.
 * Run Status: Accepted!
 * Program Runtime: 36 milli secs
 * Progress: 190/190 test cases passed.
 *
 * */

#include "../ojbst/ojbst.h"

namespace algorithm {

bool IsSymmetric(TreeNode * root1, TreeNode * root2) {
  if (root1 == NULL && root2 == NULL) return true;
  if (root1 == NULL || root2 == NULL) return false;

  if (root1->val != root2->val) return false;
  if (IsSymmetric(root1->left, root2->right) &&
      IsSymmetric(root1->right, root2->left)) return true;
  return false;
}

bool IsSymmetric(TreeNode * root) {
  if (root == NULL) return true;
  return IsSymmetric(root->left, root->right);
}

}  // namespace algorithm

namespace twice {
using namespace algorithm;

bool Mirror2(TreeNode * root1, TreeNode * root2) {
  std::stack<std::pair<TreeNode *, TreeNode *> > stack;
  while (root1 && root2) {
    if (root1->val != root2->val) return false;
    stack.push(std::make_pair(root1, root2));
    root1 = root1->left;
    root2 = root2->right;
  }
  if (root1 != NULL || root2 != NULL) return false;
  while (!stack.empty()) {
    std::pair<TreeNode *, TreeNode *> t = stack.top();
    stack.pop();
    root1 = t.first->right;
    root2 = t.second->left;
    while (root1 && root2) {
      if (root1->val != root2->val) return false;
      stack.push(std::make_pair(root1, root2));
      root1 = root1->left;
      root2 = root2->right;
    }
    if (root1 != NULL || root2 != NULL) return false;
  }
  return true;
}
}  // namespace twice

namespace third {
bool IsSym(TreeNode * root1, TreeNode * root2) {
  std::stack<std::pair<TreeNode *, TreeNode *> > stack;
  while (root1 != NULL || root2 != NULL) {
    if (root1 == NULL || root2 == NULL || root1->val != root2->val) return false;
    stack.push(std::make_pair(root1, root2));
    root1 = root1->left;
    root2 = root2->right;
  }
  while (!stack.empty()) {
    std::pair<TreeNode *, TreeNode *> t = stack.top();
    stack.pop();
    TreeNode * r1 = t.first->right;
    TreeNode * r2 = t.second->left;
    while (r1 != NULL || r2 != NULL) {
      if (r1 == NULL || r2 ==NULL || r1->val != r2->val) return false;
      stack.push(std::make_pair(r1, r2));
      r1 = r1->left;
      r2 = r2->right;
    }
  }
  return true;
}

bool IsSym(TreeNode * root) {
  if (root == NULL) return true;
  return IsSym(root->left, root->right);
}
}  // namespace third

namespace iter {
bool isSymmetric(TreeNode* root) {
  if (root == NULL || (root->left == NULL && root->right == NULL)) return true;

  TreeNode * p = root->left;
  TreeNode * q = root->right;
  std::stack<std::pair<TreeNode *, TreeNode *> > stack;
  while (p != NULL && q != NULL) {
      if (p->val != q->val) return false;
      stack.push(std::make_pair(p, q));
      p = p->left;
      q = q->right;
  }
  if (p != NULL || q != NULL) return false;

  while (stack.size()) {
      std::pair<TreeNode *, TreeNode *> cur = stack.top();
      stack.pop();
      p = cur.first;
      q = cur.second;

      p = p->right;
      q = q->left;
      while (p != NULL && q != NULL) {
          if (p->val != q->val) return false;
          stack.push(std::make_pair(p, q));
          p = p->left;
          q = q->right;
      }
      if (p != NULL || q != NULL) return false;
  }
  return true;
}
}  // namespace iter

using namespace algorithm;



int main(int argc, char** argv) {

  return 0;
}
