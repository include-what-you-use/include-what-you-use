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

// Helper macro to test strip functions on the form:
// bool op(string *s, string substr);
#define TEST_STRIP_OP(expected_res, expected, op, in, substr) \
  {                                                           \
    string str = in;                                          \
    bool res = op(&str, substr);                              \
    EXPECT_EQ(expected_res, res);                             \
    EXPECT_EQ(string(expected), str);                         \
  }

TEST(IwyuStringUtilTest, StartsWith) {
  EXPECT_TRUE(StartsWith("", ""));
  EXPECT_TRUE(StartsWith("foo", ""));
  EXPECT_TRUE(StartsWith("foo", "f"));
  EXPECT_TRUE(StartsWith("foo", "fo"));
  EXPECT_TRUE(StartsWith("foo", "foo"));

  EXPECT_FALSE(StartsWith("", "bar"));
  EXPECT_FALSE(StartsWith("foo", "bar"));
  EXPECT_FALSE(StartsWith("foo", "o"));
}

TEST(IwyuStringUtilTest, EndsWith) {
  EXPECT_TRUE(EndsWith("", ""));
  EXPECT_TRUE(EndsWith("foo", ""));
  EXPECT_TRUE(EndsWith("foo", "o"));
  EXPECT_TRUE(EndsWith("foo", "oo"));
  EXPECT_TRUE(EndsWith("foo", "foo"));

  EXPECT_FALSE(EndsWith("", "bar"));
  EXPECT_FALSE(EndsWith("foo", "bar"));
  EXPECT_FALSE(EndsWith("foo", "f"));
  EXPECT_FALSE(EndsWith("foo", "fooo"));
}

TEST(IwyuStringUtilTest, StripLeft) {
  // Test no-ops.
  TEST_STRIP_OP(false, "", StripLeft, "", "abc");
  TEST_STRIP_OP(false, "abc", StripLeft, "abc", "b");
  TEST_STRIP_OP(false, "abc", StripLeft, "abc", "bc");

  // Test actual effects.
  TEST_STRIP_OP(true, "", StripLeft, "", "");
  TEST_STRIP_OP(true, "abc", StripLeft, "abc", "");
  TEST_STRIP_OP(true, "bc", StripLeft, "abc", "a");
  TEST_STRIP_OP(true, "c", StripLeft, "abc", "ab");
  TEST_STRIP_OP(true, "", StripLeft, "abc", "abc");
}

TEST(IwyuStringUtilTest, StripRight) {
  // Test no-ops.
  TEST_STRIP_OP(false, "", StripRight, "", "abc");
  TEST_STRIP_OP(false, "abc", StripRight, "abc", "b");
  TEST_STRIP_OP(false, "abc", StripRight, "abc", "ab");

  // Test actual effects.
  TEST_STRIP_OP(true, "", StripRight, "", "");
  TEST_STRIP_OP(true, "abc", StripRight, "abc", "");
  TEST_STRIP_OP(true, "ab", StripRight, "abc", "c");
  TEST_STRIP_OP(true, "a", StripRight, "abc", "bc");
  TEST_STRIP_OP(true, "", StripRight, "abc", "abc");
}

TEST(IwyuStringUtilTest, StripPast) {
  // Test no-ops.
  TEST_STRIP_OP(false, "", StripPast, "", "abc");
  TEST_STRIP_OP(false, "abc", StripPast, "abc", "d");

  // Surprising semantics: stripping past the empty string succeeds, but
  // does nothing.
  TEST_STRIP_OP(true, "abc", StripPast, "abc", "");

  // Test actual effects.
  TEST_STRIP_OP(true, "", StripPast, "", "");
  TEST_STRIP_OP(true, "c", StripPast, "abc", "b");
  TEST_STRIP_OP(true, "", StripPast, "abc", "bc");
  TEST_STRIP_OP(true, "", StripPast, "abc", "abc");
  TEST_STRIP_OP(true, "ghi", StripPast, "abc def ghi", "def ");

  // Documentation example.
  TEST_STRIP_OP(true, " a hat!", StripPast, "What a hat!", "hat");
}

TEST(IwyuStringUtilTest, Ellipsize) {
  EXPECT_EQ("", Ellipsize("", 100));

  // Surprising semantics: if we ask for less than 6 chars we get an empty
  // string.
  EXPECT_EQ("", Ellipsize("falafel", 5));

  EXPECT_EQ("falafel", Ellipsize("falafel", 8));
  EXPECT_EQ("falafel", Ellipsize("falafel", 7));
  EXPECT_EQ("fal...", Ellipsize("falafel", 6));
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

TEST(IwyuStringUtilTest, FormatISO8601) {
  EXPECT_EQ("1970-01-01T00:00:00Z", FormatISO8601(0));
  EXPECT_EQ("2026-01-24T21:04:04Z", FormatISO8601(1769288644));
}

TEST(IwyuStringUtilTest, CollapseRepeated) {
  EXPECT_EQ("a,b", CollapseRepeated("a,,,,,,,,b", ','));
  EXPECT_EQ("a b c d e\tf", CollapseRepeated("a   b c  d    e\tf", ' '));
  EXPECT_EQ("a,b,c,d,e", CollapseRepeated("a,b,c,d,e", ','));
  EXPECT_EQ("a,b,", CollapseRepeated("a,b,", ','));
}

}  // namespace
}  // namespace include_what_you_use
