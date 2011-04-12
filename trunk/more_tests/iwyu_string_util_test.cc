//===--- iwyu_string_util_test.cpp - test iwyu_string_util.h -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


// Tests for the iwyu_string_util module.

#include <string>

#include "iwyu_string_util.h"

// TODO(user): using string for MOE.

namespace iwyu = include_what_you_use;
using iwyu::StripWhiteSpaceLeft;
using iwyu::StripWhiteSpaceRight;
using iwyu::StripWhiteSpace;

#define TEST_OPERATION(op, in, expected_out) { \
  string str = in; \
  op(&str); \
  EXPECT_EQ(string(expected_out), str); \
}

TEST(IwyuStringUtilTest, StripWhiteSpaceLeft) {
  TEST_OPERATION(StripWhiteSpaceLeft, "", "");
  TEST_OPERATION(StripWhiteSpaceLeft, " ", "");
  TEST_OPERATION(StripWhiteSpaceLeft, "abc", "abc");
  TEST_OPERATION(StripWhiteSpaceLeft, " abc", "abc");
  TEST_OPERATION(StripWhiteSpaceLeft, " abc ", "abc ");
}

TEST(IwyuStringUtilTest, StripWhiteSpaceRight) {
  TEST_OPERATION(StripWhiteSpaceRight, "", "");
  TEST_OPERATION(StripWhiteSpaceRight, " ", "");
  TEST_OPERATION(StripWhiteSpaceRight, "abc", "abc");
  TEST_OPERATION(StripWhiteSpaceRight, "abc ", "abc");
  TEST_OPERATION(StripWhiteSpaceRight, " abc ", " abc");
}

TEST(IwyuStringUtilTest, StripWhiteSpace) {
  TEST_OPERATION(StripWhiteSpace, "", "");
  TEST_OPERATION(StripWhiteSpace, " ", "");
  TEST_OPERATION(StripWhiteSpace, "abc", "abc");
  TEST_OPERATION(StripWhiteSpace, " abc", "abc");
  TEST_OPERATION(StripWhiteSpace, "abc ", "abc");
  TEST_OPERATION(StripWhiteSpace, " abc ", "abc");
}

int main() {
  return RUN_ALL_TESTS();
}
