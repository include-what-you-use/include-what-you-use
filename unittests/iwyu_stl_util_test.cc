//===--- iwyu_stl_util_test.cc - test iwyu_stl_util.h ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests for the iwyu_stl_util module.

#include "iwyu_stl_util.h"

#include <set>

#include "gtest/gtest.h"

namespace include_what_you_use {
using std::set;

namespace {

TEST(IwyuSTLUtilTest, BinaryUnion) {
  set<int> s1{1, 2, 3};
  set<int> s2{6, 3, 4, 5};

  set<int> expected{1, 2, 3, 4, 5, 6};
  EXPECT_EQ(expected, Union(s1, s2));
}

TEST(IwyuSTLUtilTest, VariadicUnion) {
  set<int> s1{1, 2, 3};
  set<int> s2{6, 3, 4};
  set<int> s3{5, 7, 8};
  set<int> s4{9, 0};

  set<int> expected{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  EXPECT_EQ(expected, Union(s1, s2, s3, s4));
}

}  // namespace
}  // namespace include_what_you_use
