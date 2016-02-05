// Copyright 2013 Jike Inc. All Rights Reserved.
// Author: Liqiang Guo(guoliqiang@jike.com)
// I just want to GH to hss~
// Date  : 2013-09-23 15:58:29
// File  : code.cc
// Brief :

/*
Divide two integers without using multiplication, division and mod operator.
不能用除法
*/

#include <algorithm>
#include <cmath>
#include "base/public/logging.h"

namespace algorithm {

int divide(int dividend, int divisor) {
  int num = 0;
  // for -2147483648 abs will return -2147483648
  // I dont know why??
  long long dd = fabs((long double)dividend);
  long long dr = fabs((long double)divisor);
  while (dr <= dd) {
    int k = 1;
    while ((dr << k) <= dd && (dr << k) > 0) {
      k++;
      LOG(INFO) << (dr << k);
    }
    k--;
    dd -= (dr << k);
    num += (1 << k);
  }
  return ((dividend ^ divisor) >> 31) ? -num : num;
}

}  // namespace algorithm

namespace twice {

long long int DiviR(long long dividend, long long int divisor) {
  if (divisor > dividend) return 0;
  if (divisor == dividend) return 1;
  int rs = 0;
  while (dividend >= divisor) {
    long long int num = 1;
    long long int t = divisor;
    while ((t << 1) < dividend) {
      t <<= 1;
      num <<= 1;
    }
    rs += num;
    dividend -= t;
  }
  return rs;
}

long long int Divi(long long int dividend, long long int divisor) {
  if (divisor > dividend) return 0;
  if (divisor == dividend) return 1;
  long long int num = 1;
  long long int t = divisor;
  while ((t << 1) < dividend) {
      t <<= 1;
      num <<= 1;
  }
  return num += Divi(dividend - t, divisor);
  
}
class Solution {
 public:
   int divide(int dividend, int divisor) {
     bool minus = ((dividend ^ divisor) >> 31) ? true : false;
     int re = Divi(fabs(dividend), fabs(divisor));
     return minus ? -re : re;
  }
};
}  // namespace twice

namespace NB {
class Solution {
 public:
  int divide(int dividend, int divisor) {
    long long a = fabs((double)dividend);;
    long long b = fabs((double)divisor);

    long long ret = 0;
    while (a >= b) {
      long long c = b;
      for (int i = 0; a >= c; ++i, c <<= 1) {
        a -= c;
        ret += 1 << i;
      }
    }
    return ((dividend^divisor)>>31) ? (-ret) : (ret);
  }
};
}  // namespace NB

namespace my {
 int divide(int dividend, int divisor) {
        bool minus = ((dividend < 0 && divisor > 0) || (dividend > 0 && divisor < 0)) ? true : false;
        long long int dr = fabs(divisor);
        long long int dd = fabs(dividend);

        long long int ans = 0;
        while (dd >= dr) {
            long long int t = 0;
            while((dr << t) <= dd) t++;
            t -= 1;
            ans += ((long long int)1 << t);  // bug fixed
            dd -= (dr << t);
        }
        if (ans > INT_MAX) return minus ? INT_MIN : INT_MAX;
        return minus ? -ans : ans;
    }
}  // namespace my

using namespace algorithm;

int main(int argc, char** argv) {
  LOG(INFO) << "7/3:" << my::divide(-2147483648, -1);
  return 0;
}
