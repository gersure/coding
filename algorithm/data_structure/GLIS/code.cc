// Copyright 2013 Jike Inc. All Rights Reserved.
// Author: Liqiang Guo(guoliqiang@jike.com)
// I just want to GH to hss~
// Date  : 2013-11-05 00:38:32
// File  : code.cc
// Brief :

#include "base/public/common_head.h"

namespace algorithm {

/*
 * ptr 不减序，如果ptr中存在等于k的元素，返回其下表
 * 否则返回比ｋ稍大数的下表，如果ptr中所有元素都小于ｋ，返回ｎ
 * */
int BsearchEI(int * ptr, int n, int k) {
  int b = 0;
  int e = n - 1;
  while (b <= e) {
    int mid = b + (e - b) /2;
    if (ptr[mid] < k) b = mid + 1;
    else e = mid - 1;
  }
  return b;
}

/*
 * ptr不减序，始终返回比ｋ稍微大的元素的下表，如果不存在返回ｎ
 * */

int BsearchI(int * ptr, int n, int k) {
  int b = 0;
  int e = n - 1;
  while (b <= e) {
    int mid = b + (e - b) / 2;
    if (ptr[mid] > k && (mid == 0 || ptr[mid - 1] <= k)) return mid;
    else {
      if (ptr[mid] <= k) b = mid + 1;
      else e = mid - 1;
    }
  }
  return b;
}

/*
 * ptr不增序，若ptr中存在ｋ返回其下表，
 * 否则返回稍微小于ｋ的下标，如果不存在返回ｎ
 * */
int BsearchED(int * ptr, int n, int k) {
  int b = 0;
  int e = n - 1;
  while (b <= e) {
    int mid = b + (e - b) / 2;
    if (ptr[mid] > k) b = mid + 1;
    else e = mid - 1;
  }
  return b;
}

/*
 * ptr不增序，始终返回比ｋ稍微小的数的下表，如果不存在返回ｎ
 * */

int BsearchD(int * ptr, int n, int k) {
  int b = 0;
  int e = n - 1;
  while (b <= e) {
    int mid = b + (e - b) / 2;
    if (ptr[mid] < k && (k == 0 || ptr[mid - 1] >= k)) return mid;
    else {
      if (ptr[mid] >= k) b = mid + 1;
      else e = mid - 1;
    }
  }
  return b;
}

int GLSI(std::vector<int> & v) {
  std::vector<int> dp(v.size(), 0);
  int size = 0;
  for (int i = 0; i < v.size(); i++) {
    // int t = BsearchI(&dp[0], size, v[i]);  // 递增序列
    // int t = BsearchEI(&dp[0], size, v[i]); // 不减序列
    // int t = BsearchED(&dp[0], size, v[i]);  // 递减序列
    int t = BsearchD(&dp[0], size, v[i]);      // 不增序列
    // LOG(INFO) << t;
    if (t == size) {
      size++;
    }
    dp[t] = v[i];
    LOG(INFO) << JoinVector(dp);
  }
  return size;
}

}  // namespace algorithm

using namespace algorithm;


int main(int argc, char** argv) {
  std::vector<int> vec;
  vec.push_back(3);
  vec.push_back(2);
  vec.push_back(2);
  vec.push_back(2);
  vec.push_back(2);
  vec.push_back(1);
  vec.push_back(4);
  vec.push_back(6);
  vec.push_back(2);
  vec.push_back(8);
  LOG(INFO) << GLSI(vec);
  return 0;
  
  
  
  
  std::vector<int> v;
  v.push_back(1);
  v.push_back(2);
  v.push_back(2);
  v.push_back(2);
  v.push_back(3);
  v.push_back(5);
  LOG(INFO) << "BsearchEI";
  LOG(INFO) << BsearchEI(&(v[0]), v.size(), 1);
  LOG(INFO) << BsearchEI(&(v[0]), v.size(), 0);
  LOG(INFO) << BsearchEI(&(v[0]), v.size(), 5);
  LOG(INFO) << BsearchEI(&(v[0]), v.size(), 6);
  LOG(INFO) << BsearchEI(&(v[0]), v.size(), 3);
  LOG(INFO) << BsearchEI(&(v[0]), v.size(), 4);
  LOG(INFO) << "BsearchI";
  LOG(INFO) << BsearchI(&(v[0]), v.size(), 1);
  LOG(INFO) << BsearchI(&(v[0]), v.size(), 0);
  LOG(INFO) << BsearchI(&(v[0]), v.size(), 4);
  LOG(INFO) << BsearchI(&(v[0]), v.size(), 5);
  LOG(INFO) << BsearchI(&(v[0]), v.size(), 2);
  LOG(INFO) << BsearchI(&(v[0]), v.size(), 3);
  v.clear();
  v.push_back(5);
  v.push_back(3);
  v.push_back(2);
  v.push_back(2);
  v.push_back(2);
  v.push_back(2);
  v.push_back(1);
  LOG(INFO) << "BsearchED";
  LOG(INFO) << BsearchED(&(v[0]), v.size(), 1);
  LOG(INFO) << BsearchED(&(v[0]), v.size(), 0);
  LOG(INFO) << BsearchED(&(v[0]), v.size(), 4);
  LOG(INFO) << BsearchED(&(v[0]), v.size(), 5);
  LOG(INFO) << BsearchED(&(v[0]), v.size(), 2);
  LOG(INFO) << BsearchED(&(v[0]), v.size(), 3);
  
  LOG(INFO) << "BsearchD";
  LOG(INFO) << BsearchD(&(v[0]), v.size(), 1);
  LOG(INFO) << BsearchD(&(v[0]), v.size(), 0);
  LOG(INFO) << BsearchD(&(v[0]), v.size(), 4);
  LOG(INFO) << BsearchD(&(v[0]), v.size(), 5);
  LOG(INFO) << BsearchD(&(v[0]), v.size(), 2);
  LOG(INFO) << BsearchD(&(v[0]), v.size(), 3);
  return 0;
}