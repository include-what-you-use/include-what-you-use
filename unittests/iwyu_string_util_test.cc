//===--- iwyu_string_util_test.cc - test iwyu_string_util.h ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests for the iwyu_string_util module.

#include "iwyu_string_util.h"

#include <string>

#include "gtest/gtest.h"
#include "iwyu_test_helpers.h"

namespace include_what_you_use {
using std::string;
using std::vector;

namespace {

// Helper macro to test whitespace strip functions on the form:
// void op(string* s);
#define TEST_STRIP_WHITESPACE_OP(expected, op, in) \
  {                                                \
    string str = in;                               \
    op(&str);                                      \
    EXPECT_EQ(string(expected), str);              \
  }

TEST(IwyuStringUtilTest, StripWhiteSpaceLeft) {
  TEST_STRIP_WHITESPACE_OP("", StripWhiteSpaceLeft, "");
  TEST_STRIP_WHITESPACE_OP("", StripWhiteSpaceLeft, " ");
  TEST_STRIP_WHITESPACE_OP("abc", StripWhiteSpaceLeft, "abc");
  TEST_STRIP_WHITESPACE_OP("abc", StripWhiteSpaceLeft, " abc");
  TEST_STRIP_WHITESPACE_OP("abc ", StripWhiteSpaceLeft, " abc ");
}

TEST(IwyuStringUtilTest, StripWhiteSpaceRight) {
  TEST_STRIP_WHITESPACE_OP("", StripWhiteSpaceRight, "");
  TEST_STRIP_WHITESPACE_OP("", StripWhiteSpaceRight, " ");
  TEST_STRIP_WHITESPACE_OP("abc", StripWhiteSpaceRight, "abc");
  TEST_STRIP_WHITESPACE_OP("abc", StripWhiteSpaceRight, "abc ");
  TEST_STRIP_WHITESPACE_OP(" abc", StripWhiteSpaceRight, " abc ");
}

TEST(IwyuStringUtilTest, StripWhiteSpace) {
  TEST_STRIP_WHITESPACE_OP("", StripWhiteSpace, "");
  TEST_STRIP_WHITESPACE_OP("", StripWhiteSpace, " ");
  TEST_STRIP_WHITESPACE_OP("abc", StripWhiteSpace, "abc");
  TEST_STRIP_WHITESPACE_OP("abc", StripWhiteSpace, " abc");
  TEST_STRIP_WHITESPACE_OP("abc", StripWhiteSpace, "abc ");
  TEST_STRIP_WHITESPACE_OP("abc", StripWhiteSpace, " abc ");
}

TEST(IwyuStringUtilTest, SplitOnWhiteSpace) {
  string in = "this is a test";
  vector<string> out = SplitOnWhiteSpace(in, 0);
  EXPECT_EQ("", ArrayDiff({"this", "is", "a", "test"}, out));

  in = "this is a test";
  out = SplitOnWhiteSpace(in, 2);
  EXPECT_EQ("", ArrayDiff({"this", "is"}, out));

  in = "this is\ta    test";
  out = SplitOnWhiteSpace(in, 0);
  EXPECT_EQ("", ArrayDiff({"this", "is", "a", "test"}, out));

  in = " this is a test ";
  out = SplitOnWhiteSpace(in, 0);
  EXPECT_EQ("", ArrayDiff({"this", "is", "a", "test"}, out));
}

TEST(IwyuStringUtilTest, SplitOnWhiteSpacePreservingQuotes) {
  string in = "this is <a test>";
  vector<string> out = SplitOnWhiteSpacePreservingQuotes(in, 0);
  EXPECT_EQ("", ArrayDiff({"this", "is", "<a test>"}, out));

  in = "this <is a> test";
  out = SplitOnWhiteSpacePreservingQuotes(in, 2);
  EXPECT_EQ("", ArrayDiff({"this", "<is a>"}, out));

  in = "\"this is\"\ta    test";
  out = SplitOnWhiteSpacePreservingQuotes(in, 0);
  EXPECT_EQ("", ArrayDiff({"\"this is\"", "a", "test"}, out));

  in = " this is \"a test\" ";
  out = SplitOnWhiteSpacePreservingQuotes(in, 0);
  EXPECT_EQ("", ArrayDiff({"this", "is", "\"a test\""}, out));
}

}  // namespace
}  // namespace include_what_you_use
