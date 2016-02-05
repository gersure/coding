// Copyright 2013 Jike Inc. All Rights Reserved.
// Author: Liqiang Guo(guoliqiang@jike.com)
// I just want to GH to hss~
// Date  : 2013-09-24 20:00:06
// File  : code.cc
// Brief :

/*
Given a collection of candidate numbers (C) and a target number (T), find all unique combinations in C where the candidate numbers sums to T.

Each number in C may only be used once in the combination.

Note:
All numbers (including target) will be positive integers.
Elements in a combination (a1, a2, … , ak) must be in non-descending order. (ie, a1 ≤ a2 ≤ … ≤ ak).
The solution set must not contain duplicate combinations.
For example,given candidate set 10,1,2,7,6,1,5 and target 8, 
A solution set is: 
[1, 7] 
[1, 2, 5] 
[2, 6] 
[1, 1, 6]

*/

/*
 * 0 - 1 背包问题
 *
 *
 * Run Status: Accepted!
 * Program Runtime: 4 milli secs
 * Progress: 22/22 test cases passed.
 * Run Status: Accepted!
 * Program Runtime: 80 milli secs
 * Progress: 172/172 test cases passed.
 *
 * */

#include <vector>
#include <set>
#include <iostream>
#include "base/public/logging.h"
#include "base/public/string_util.h"

namespace algorithm {
void DP(std::vector<int> & candi, int n, int **dp) {
  for (int i = 0; i < candi.size(); i++) dp[0][i] = 0;
  for (int i = 0; i <= n ; i++) dp[i][0] = 0;

  for (int i = 1; i < candi.size(); i++) {
    for (int j = 1; j <= n; j ++) {
      if (j < candi[i]) dp[j][i] = dp[j][i - 1];
      else {
        dp[j][i] = std::max(dp[j][i - 1], candi[i] + dp[j - candi[i]][i - 1]);
      }
      // LOG(INFO) << "dp[" << i << "][" << j << "]" << dp[i][j];
    }
  }
}

void Out(int ** dp, int n, int m) {
  for (int i = 0; i <= n; i++) {
    for (int j = 0; j < m; j++) {
      std::cout << dp[i][j] << " ";
    }
    std::cout << std::endl;
  }
}

void Trace(std::vector<int> & candi, int ** dp,
           std::vector<int> & path,
           int i, int j,
           std::set<std::vector<int> > & rs) {
  if (i == 0 || j == 0) {
    std::vector<int> bar = path;
    std::sort(bar.begin(), bar.end());
    rs.insert(bar);
  } else {
    if (dp[j][i - 1] == dp[j][i]) {
      Trace(candi, dp, path, i - 1, dp[j][i], rs);
    }
    if (j >= candi[i] && dp[j - candi[i]][i - 1] + candi[i] == dp[j][i]) {
      path.push_back(candi[i]);
      Trace(candi, dp, path, i - 1, dp[j][i] - candi[i], rs);
      path.pop_back();
    }
  }
}

std::vector<std::vector<int> > CombinationSum(std::vector<int> candi, int n) {
  candi.push_back(-1);
  std::sort(candi.begin(), candi.end());
  int ** dp = new int * [n + 1];
  for (int i = 0; i <= n; i++) {
    dp[i] = new int[candi.size()];
  }
  DP(candi, n, dp);
  Out(dp, n, candi.size());
  std::vector<std::vector<int> > rs;
  std::set<std::vector<int> > tmp;
  std::vector<int> path;
  if (dp[n][candi.size() - 1] == n) Trace(candi, dp, path, candi.size() - 1, n, tmp);
  for (std::set<std::vector<int> >::iterator i = tmp.begin(); i != tmp.end(); i++) {
    rs.push_back(*i);
  }
  for (int i = 0; i <=n; i++) delete [] dp[i];
  delete [] dp;
  return rs;
}

}  // namespace algorithm

using namespace algorithm;

namespace twice {

using namespace std;

void Combine(std::vector<int> & num, int target, int k, std::vector<int> & path,
             std::set<std::vector<int> > & rs, int cur) {
  if (k == num.size()) {
      int t = 0;
      std::vector<int> foo;
      for (int i = 0; i < path.size(); i++) {
          if (path[i] == 1) {
              t += num[i];
              foo.push_back(num[i]);
          }
          if (t >= target) break;
      }
      if (t == target) rs.insert(foo);
  } else {
      for (int i = 0; i < 2; i++) {
          path[k] = i;
          if (cur + path[k] * num[k] > target) continue;
          Combine(num, target, k + 1, path, rs, cur + path[k] * num[k]);
      }
  }
}

// 只能选一次的
void Combine2(std::vector<int> & num, int target, int k, std::vector<int> & path,
             std::set<std::vector<int> > & rs, int cur) {
  if (cur == target) {
    rs.insert(path);
    return;
  }
  if (cur > target || k == num.size()) return;

  Combine2(num, target, k + 1, path, rs, cur);
  path.push_back(num[k]);
  Combine2(num, target, k + 1, path, rs, cur + num[k]);
  path.pop_back();
}

class Solution {
public:
    vector<vector<int> > combinationSum2(vector<int> &num, int target) {
        std::sort(num.begin(), num.end());
        std::set<std::vector<int> > tmp;
        // method1:
        // std::vector<int> path(num.size(), 0);
        // Combine(num, target, 0, path, tmp, 0);
        
        // method2:
        std::vector<int> path;
        Combine2(num, target, 0, path, tmp, 0);
        return std::vector<std::vector<int> > (tmp.begin(), tmp.end());
    }
};
}  // namespace twice


int main(int argc, char** argv) {
  std::vector<int> candi;
  // candi.push_back(10);
  candi.push_back(2);
  /*
  candi.push_back(2);
  candi.push_back(7);
  candi.push_back(6);
  candi.push_back(1);
  candi.push_back(5);
  */
  int n = 1;
  twice::Solution s;
  // std::vector<std::vector<int> > rs = CombinationSum(candi, n);
  std::vector<std::vector<int> > rs = s.combinationSum2(candi, n);
  for (int i = 0; i < rs.size(); i++) {
    LOG(INFO) << JoinVector(rs[i]);
  }
  return 0;
}
