//===--- std_symbol_mapping.cc - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests the internal IWYU standard library symbol mapping. In particular, tests
// that the mapping works with types in the fwd-decl context.

#include "tests/cxx/std_symbol_mapping-direct.h"

// IWYU: pair is...*<utility>
std::pair<int, int>* p;

template <typename T>
// IWYU: pair is...*<utility>
std::pair<T, T> foo();

void Fn() {
  // IWYU: std::get(std::pair<:1, :2> &) is...*<utility>
  std::get<0>(*p);
  // IWYU: std::get(std::pair<:1, :2> &&) is...*<utility>
  // IWYU: std::move(:0 &&) is...*<utility>
  std::get<1>(std::move(*p));
  // IWYU: std::pair is...*<utility>
  extern const std::pair<int, char> pair2;
  // IWYU: std::get(const std::pair<:0, :1> &) is...*<utility>
  (void)std::get<int>(pair2);
  // IWYU: std::get(const std::pair<:1, :0> &) is...*<utility>
  (void)std::get<char>(pair2);

  // IWYU: std::array is...*<array>
  extern const std::array<int, 7> std_arr;
  // IWYU: std::get(const std::array<:1, :2> &&) is...*<array>
  // IWYU: std::move(:0 &&) is...*<utility>
  (void)std::get<1>(std::move(std_arr));

  int arr1[5] = {}, arr2[5] = {}, i1 = 0, i2 = 0;
  // IWYU: std::move(:0, :0, :1) is...*<algorithm>
  std::move(arr1, arr1 + 5, arr2);

  // IWYU: std::swap(:0 &, :0 &) is...*<utility>
  std::swap(i1, i2);
  // IWYU: std::swap(:0 (&)[:1], :0 (&)[:1]) is...*<utility>
  std::swap(arr1, arr2);

  // IWYU: std::function is...*<functional>
  extern std::function<void()> f1, f2;
  // IWYU: std::swap(std::function<:0 (:1...)> &, std::function<:0 (:1...)> &) is...*<functional>
  swap(f1, f2);

  // IWYU: std::valarray is...*<valarray>
  std::valarray<int> va;
  // IWYU: std::pow(const std::valarray<:0> &, const std::valarray<:0>::value_type &) is...*<valarray>
  (void)pow(va, 2);
  // IWYU: std::pow(float, float) is...*<cmath>
  (void)std::pow(2.0F, 3.0F);
  // IWYU: std::pow(double, double) is...*<cmath>
  (void)std::pow(2.0, 3.0);
  // IWYU: std::pow(long double, long double) is...*<cmath>
  (void)std::pow(2.0L, 3.0L);
}

/**** IWYU_SUMMARY

tests/cxx/std_symbol_mapping.cc should add these lines:
#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <utility>
#include <valarray>

tests/cxx/std_symbol_mapping.cc should remove these lines:
- #include "tests/cxx/std_symbol_mapping-direct.h"  // lines XX-XX

The full include-list for tests/cxx/std_symbol_mapping.cc:
#include <algorithm>  // for move
#include <array>  // for array, get
#include <cmath>  // for pow
#include <functional>  // for function, swap
#include <utility>  // for get, move, pair, swap
#include <valarray>  // for pow, valarray

***** IWYU_SUMMARY */
