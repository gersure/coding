// Copyright 2013 Jike Inc. All Rights Reserved.
// Author: Liqiang Guo(guoliqiang@jike.com)
// I just want to GH to hss~
// Date  : 2013-09-20 23:21:53
// File  : code.h
// Brief :

#ifndef  __CODE_H_
#define  __CODE_H_

#include <vector>
#include "base/public/logging.h"

namespace algorithm {
std::string zigzag(std::string text, int n) {
  if (n == 1) return text;
  std::vector<std::string> foo(n, std::string(""));
  int index = 0;
  int bar = 0;
  bool flag = false;
  for (int i = 0; i < text.size(); i++) {
    if (flag) {
      foo[n - 2 - bar].append(&text[i], 1);
    } else {
      foo[bar].append(&text[i], 1);
    }
    index = (index + 1) % (2 * n - 2);
    bar = index % n;
    if (n > 2 && bar == 0) flag = (!flag);
  }
  std::string rs = "";
  for (int i = 0; i < foo.size(); i++) {
    rs += foo[i];
  }
  return rs;
}
}  // namespace algorithm

#endif  //__CODE_H_