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

TEST(IwyuSTLUtilTest, UnionSetSemantics) {
  // Make sure we eliminate duplicates
  set<int> s1{1, 2};
  set<int> s2{1, 2};
  EXPECT_EQ(set<int>({1, 2}), Union(s1, s2));
  set<int> s3{1, 2};
  EXPECT_EQ(set<int>({1, 2}), Union(s1, s2, s3));
}

TEST(IwyuSTLUtilTest, Union) {
  set<int> s1{1, 2, 3};
  set<int> s2{4, 5, 6};
  set<int> s3{7, 8, 9};
  set<int> s4{10, 11, 12};

  // Union requires at least one set.
  EXPECT_EQ(set<int>({1, 2, 3}), Union(s1));
  EXPECT_EQ(set<int>({1, 2, 3, 4, 5, 6}), Union(s1, s2));

  // Odd number of sets.
  EXPECT_EQ(set<int>({1, 2, 3, 4, 5, 6, 7, 8, 9}), Union(s1, s2, s3));

  // Even number of sets >2.
  EXPECT_EQ(set<int>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}),
            Union(s1, s2, s3, s4));

  // Union of set rvalues.
  EXPECT_EQ(set<int>({1, 2, 3, 4, 5, 6}), Union(std::move(s1), std::move(s2)));

  // Union of mixed lvalues and rvalues.
  EXPECT_EQ(set<int>({7, 8, 9, 10, 11, 12}), Union(std::move(s3), s4));
}

}  // namespace
}  // namespace include_what_you_use
