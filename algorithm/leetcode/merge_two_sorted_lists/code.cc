// Copyright 2013 Jike Inc. All Rights Reserved.
// Author: Liqiang Guo(guoliqiang@jike.com)
// I just want to GH to hss~
// Date  : 2013-09-26 20:17:34
// File  : code.cc
// Brief :
//
/*
 * Run Status: Accepted!
 * Program Runtime: 12 milli secs
 * Progress: 9/9 test cases passed.
 * Run Status: Accepted!
 * Program Runtime: 60 milli secs
 * Progress: 208/208 test cases passed.
 *
 * */
#include "base/public/logging.h"

namespace algorithm {

/*
 * 多看优秀的代码，真好～
 * disscuss 的代码质量很好
 * 
 * head pre的解决方法很巧妙
 * */

ListNode * MergeTwoListNB(ListNode * l1, ListNode * l2) {
  ListNode * head = NULL;
  ListNode * * pre = &head;

  while (l1 != NULL && l2 != NULL) {
    ListNode * min = NULL;
    if (l1->val < l2->val) min = l1;
    else min = l2;

    *pre = min;
    pre = &((*pre)->next);
    if (min == l1) l1 = l1->next;
    if (min == l2) l2 = l2->next;
  }

  if (l1 != NULL) *pre = l1;
  if (l2 != NULL) *pre = l2;

  return head;
}

ListNode * MergeTwoList(ListNode * l1, ListNode * l2) {
  ListNode * head = NULL;
  ListNode * pre = NULL;

  while (l1 != NULL && l2 != NULL) {
    ListNode * min = NULL;
    if (l1->val < l2->val) min = l1;
    else min = l2;

    if (head == NULL) {
      head = min;
      pre = head;
    } else {
      pre->next = min;
      pre = pre->next;
    }
    if (min == l1) l1 = l1->next;
    if (min == l2) l2 = l2->next;
  }

  ListNode * last = NULL;
  if (l1 != NULL) last = l1;
  if (l2 != NULL) last = l2;
  if (head == NULL) head = last;
  else pre->next = last;

  return head;
}

}  // namespace algorithm

using namespace algorithm;


int main(int argc, char** argv) {

  return 0;
}