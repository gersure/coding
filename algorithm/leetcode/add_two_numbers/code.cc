// Copyright 2013 Jike Inc. All Rights Reserved.
// Author: Liqiang Guo(guoliqiang@jike.com)
// I just want to GH to hss~
// Date  : 2013-09-20 13:26:52
// File  : code.cc
// Brief :

/*
 * You have two numbers represented by a linked list, where each node contains a single digit.
 * The digits are stored in reverse order, such that the 1’s digit is at the head of the list.
 * Write a function that adds the two numbers and returns the sum as a linked list. 
 * EXAMPLE Input: (3 –> 1 –> 5) + (5 –> 9 –> 2) Output: 8 –> 0 –> 8
 * */

/*
 * 如果数据不是从个位数开始的则需要先翻转列表
 *
 * */

#include "base/public/common_head.h"

namespace algorithm {

struct ListNode {
  int val;
  ListNode *next;
  ListNode(int x) : val(x), next(NULL) {}
};

class Solution {
 public:
   ListNode *addTwoNumbers(ListNode *l1, ListNode *l2) {
     // Start typing your C/C++ solution below
     // DO NOT write int main() function
     int carry = 0;
     ListNode * root = NULL;
     ListNode * before = NULL;
     while (l1 != NULL && l2 != NULL) {
       int v = (l1->val + l2->val + carry) % 10;
       carry = (l1->val + l2->val + carry) / 10;
       if (root == NULL) {
         root = new ListNode(v);
         before = root;
       } else {
         before->next = new ListNode(v);
         before = before->next;
       }
       l1 = l1->next; l2 = l2->next;
     }
        
     ListNode * bar = NULL;
     if (l1 == NULL) bar = l2;
     else bar = l1;
     while (bar != NULL) {
       int v = (bar->val + carry) % 10;
       carry = (bar->val + carry) / 10;
       before->next = new ListNode(v);
       before = before->next;
       bar = bar->next;
     }
        
     while (carry > 0) {
       before->next = new ListNode(carry % 10);
       before = before->next;
       carry /= 10;
     }
     return root;
  }
};

}  // namespace algorithm


namespace NB {

/*
 * 1555 / 1555 test cases passed.
 * Status: Accepted
 * Runtime: 192 ms
 * Submitted: 0 minutes ago
 *
 * */

using namespace algorithm;

ListNode * Add(ListNode * l1, ListNode * l2) {
  ListNode * rs = NULL;
  ListNode ** tail = &rs;
  int carry = 0;
  while (l1 != NULL || l2 != NULL || carry != 0) {
    carry += (l1 == NULL ? 0 : l1->val) + (l2 == NULL ? 0 : l2->val);
    *tail = new ListNode(carry % 10);
    tail = &((*tail)->next);
    carry /= 10;

    l1 = l1 == NULL ? NULL : l1->next;
    l2 = l2 == NULL ? NULL : l2->next;
  }
  return rs;
}

}  // namespace NB


int main(int argc, char** argv) {

  return 0;
}
