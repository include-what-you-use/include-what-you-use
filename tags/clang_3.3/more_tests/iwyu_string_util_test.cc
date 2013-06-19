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


// TODO(dsturtevant): using string for MOE.

namespace iwyu = include_what_you_use;
using iwyu::SplitOnWhiteSpace;
using iwyu::SplitOnWhiteSpacePreservingQuotes;
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

TEST(IwyuStringUtilTest, SplitOnWhiteSpace) {
  string in = "this is a test";
  vector<string> out = SplitOnWhiteSpace(in, 0);
  ASSERT_EQ(4, out.size());
  EXPECT_EQ(string("this"), out[0]);
  EXPECT_EQ(string("is"), out[1]);
  EXPECT_EQ(string("a"), out[2]);
  EXPECT_EQ(string("test"), out[3]);

  in = "this is a test";
  out = SplitOnWhiteSpace(in, 2);
  ASSERT_EQ(2, out.size());
  EXPECT_EQ(string("this"), out[0]);
  EXPECT_EQ(string("is"), out[1]);

  in = "this is\ta    test";
  out = SplitOnWhiteSpace(in, 0);
  ASSERT_EQ(4, out.size());
  EXPECT_EQ(string("this"), out[0]);
  EXPECT_EQ(string("is"), out[1]);
  EXPECT_EQ(string("a"), out[2]);
  EXPECT_EQ(string("test"), out[3]);

  in = " this is a test ";
  out = SplitOnWhiteSpace(in, 0);
  ASSERT_EQ(4, out.size());
  EXPECT_EQ(string("this"), out[0]);
  EXPECT_EQ(string("is"), out[1]);
  EXPECT_EQ(string("a"), out[2]);
  EXPECT_EQ(string("test"), out[3]);
}

TEST(IwyuStringUtilTest, SplitOnWhiteSpacePreservingQuotes) {
  string in = "this is <a test>";
  vector<string> out = SplitOnWhiteSpacePreservingQuotes(in, 0);
  ASSERT_EQ(3, out.size());
  EXPECT_EQ(string("this"), out[0]);
  EXPECT_EQ(string("is"), out[1]);
  EXPECT_EQ(string("<a test>"), out[2]);

  in = "this <is a> test";
  out = SplitOnWhiteSpacePreservingQuotes(in, 2);
  ASSERT_EQ(2, out.size());
  EXPECT_EQ(string("this"), out[0]);
  EXPECT_EQ(string("<is a>"), out[1]);

  in = "\"this is\"\ta    test";
  out = SplitOnWhiteSpacePreservingQuotes(in, 0);
  ASSERT_EQ(3, out.size());
  EXPECT_EQ(string("\"this is\""), out[0]);
  EXPECT_EQ(string("a"), out[1]);
  EXPECT_EQ(string("test"), out[2]);

  in = " this is \"a test\" ";
  out = SplitOnWhiteSpacePreservingQuotes(in, 0);
  ASSERT_EQ(3, out.size());
  EXPECT_EQ(string("this"), out[0]);
  EXPECT_EQ(string("is"), out[1]);
  EXPECT_EQ(string("\"a test\""), out[2]);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
