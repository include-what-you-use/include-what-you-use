//===--- iwyu_regex_test.cc - test iwyu_regex.h ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests for the iwyu_regex module.

#include "iwyu_regex.h"

#include <string>

#include "gtest/gtest.h"

namespace include_what_you_use {

namespace {

// --- ParseRegexDialect tests ---

TEST(IwyuRegexTest, ParseRegexDialectLLVM) {
  RegexDialect dialect;
  EXPECT_TRUE(ParseRegexDialect("llvm", &dialect));
  EXPECT_EQ(RegexDialect::LLVM, dialect);
}

TEST(IwyuRegexTest, ParseRegexDialectECMAScript) {
  RegexDialect dialect;
  EXPECT_TRUE(ParseRegexDialect("ecmascript", &dialect));
  EXPECT_EQ(RegexDialect::ECMAScript, dialect);
}

TEST(IwyuRegexTest, ParseRegexDialectInvalidOrEmpty) {
  RegexDialect dialect = RegexDialect::LLVM;
  EXPECT_FALSE(ParseRegexDialect("invalid", &dialect));
  EXPECT_EQ(RegexDialect::LLVM, dialect);

  EXPECT_FALSE(ParseRegexDialect("", &dialect));
  EXPECT_EQ(RegexDialect::LLVM, dialect);
}

// --- Parameterized RegexMatch/RegexReplace tests ---
// Tests that behave identically across both dialects.

class RegexDialectTest : public ::testing::TestWithParam<RegexDialect> {};

TEST_P(RegexDialectTest, MatchExact) {
  EXPECT_TRUE(RegexMatch(GetParam(), "hello", "hello"));
}

TEST_P(RegexDialectTest, MatchNoMatch) {
  EXPECT_FALSE(RegexMatch(GetParam(), "hello", "world"));
}

TEST_P(RegexDialectTest, MatchPartialDoesNotMatch) {
  EXPECT_FALSE(RegexMatch(GetParam(), "hello world", "hello"));
}

TEST_P(RegexDialectTest, MatchWildcard) {
  EXPECT_TRUE(RegexMatch(GetParam(), "hello world", "hello.*"));
}

TEST_P(RegexDialectTest, MatchEmptyPattern) {
  EXPECT_TRUE(RegexMatch(GetParam(), "", ""));
  EXPECT_FALSE(RegexMatch(GetParam(), "hello", ""));
}

TEST_P(RegexDialectTest, MatchDigits) {
  EXPECT_TRUE(RegexMatch(GetParam(), "test123", "test[0-9]+"));
  EXPECT_FALSE(RegexMatch(GetParam(), "test", "test[0-9]+"));
}

TEST_P(RegexDialectTest, ReplaceExact) {
  std::string result =
      RegexReplace(GetParam(), "hello", "hello", "world");
  EXPECT_EQ("world", result);
}

TEST_P(RegexDialectTest, ReplaceNoMatch) {
  std::string result =
      RegexReplace(GetParam(), "hello", "xyz", "abc");
  EXPECT_EQ("hello", result);
}

INSTANTIATE_TEST_SUITE_P(AllDialects, RegexDialectTest,
                         ::testing::Values(RegexDialect::LLVM,
                                           RegexDialect::ECMAScript));

// --- Dialect-specific tests ---

TEST(IwyuRegexTest, RegexReplaceLLVMWithCapture) {
  std::string result = RegexReplace(RegexDialect::LLVM, "hello world",
                                    "(hello) (world)", "\\2 \\1");
  EXPECT_EQ("world hello", result);
}

TEST(IwyuRegexTest, RegexReplaceECMAScriptWithCapture) {
  std::string result = RegexReplace(RegexDialect::ECMAScript, "hello world",
                                    "(hello) (world)", "$2 $1");
  EXPECT_EQ("world hello", result);
}

// ECMAScript supports negative lookahead; LLVM does not.
// See issues #981, #935 and commits cf5388082266, a5d8408c5f2c.
// Note: std::regex ECMAScript mode follows the ES5 spec which only supports
// lookahead (?=...) and (?!...), not lookbehind (?<=...) and (?<!...).

TEST(IwyuRegexTest, ECMAScriptNegativeLookahead) {
  // Match "foo" not followed by "bar".
  EXPECT_TRUE(RegexMatch(RegexDialect::ECMAScript, "foobaz", "foo(?!bar).*"));
  EXPECT_FALSE(RegexMatch(RegexDialect::ECMAScript, "foobar", "foo(?!bar).*"));
}

}  // namespace
}  // namespace include_what_you_use
