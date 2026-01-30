//===--- iwyu_path_util_test.cc - test iwyu_path_util ------------- -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests iwyu_path_util.

#include "iwyu_path_util.h"

#include <string>

#include "gtest/gtest.h"

namespace include_what_you_use {
using std::string;

namespace {

TEST(GetCanonicalName, StripsKnownSuffixes) {
  EXPECT_EQ("my/path/foo", GetCanonicalName("my/path/foo.cxx"));
  EXPECT_EQ("my/path/foo", GetCanonicalName("my/path/foo.cpp"));
  EXPECT_EQ("my/path/foo", GetCanonicalName("my/path/foo.cc"));
  EXPECT_EQ("my/path/foo", GetCanonicalName("my/path/foo.h"));
  EXPECT_EQ("my/path/foo", GetCanonicalName("my/path/foo-inl.h"));
  EXPECT_EQ("my/path/foo", GetCanonicalName("my/path/foo_unittest.cc"));
  EXPECT_EQ("my/path/foo", GetCanonicalName("my/path/foo_regtest.cc"));
  EXPECT_EQ("my/path/foo", GetCanonicalName("my/path/foo_test.cc"));
  EXPECT_EQ("my/path/foo", GetCanonicalName("my/path/foo.c"));
  EXPECT_EQ("my/path/foo", GetCanonicalName("my/path/foo-inl_unittest.cc"));
  EXPECT_EQ("my/path/foo_mytest", GetCanonicalName("my/path/foo_mytest.cc"));
}

TEST(GetCanonicalName, MapsInternalToPublic) {
  EXPECT_EQ("my/public/foo", GetCanonicalName("my/internal/foo.cc"));
  EXPECT_EQ("my/public/foo", GetCanonicalName("my/public/foo.cc"));
  EXPECT_EQ("my/public/foo", GetCanonicalName("my/internal/foo.h"));
  EXPECT_EQ("my/public/foo", GetCanonicalName("my/public/foo.h"));
  EXPECT_EQ("internal/foo", GetCanonicalName("internal/foo"));
  EXPECT_EQ("path/internal_impl", GetCanonicalName("path/internal_impl.cc"));
}

TEST(ConvertToQuotedInclude, Basic) {
  EXPECT_EQ("\"foo.h\"", ConvertToQuotedInclude("foo.h"));
  EXPECT_EQ("<string.h>", ConvertToQuotedInclude("/usr/include/string.h"));
  EXPECT_EQ("<bits/stl_vector.h>",
            ConvertToQuotedInclude("/usr/include/c++/4.3/bits/stl_vector.h"));
}

TEST(IsQuotedHeaderFilename, Basic) {
  // Nominal cases.
  EXPECT_TRUE(IsQuotedHeaderFilename("\"foo.h\""));
  EXPECT_TRUE(IsQuotedHeaderFilename("\"bar/foo.h\""));
  EXPECT_TRUE(IsQuotedHeaderFilename("\"foo.inl\""));
  EXPECT_TRUE(IsQuotedHeaderFilename("\"foo.def\""));

  EXPECT_TRUE(IsQuotedHeaderFilename("<string.h>"));
  EXPECT_TRUE(IsQuotedHeaderFilename("<bits/stl_vector.h>"));
  EXPECT_TRUE(IsQuotedHeaderFilename("<string>"));

  // A bit unusual, but considered headers.
  EXPECT_TRUE(IsQuotedHeaderFilename("\"foo\""));
  EXPECT_TRUE(IsQuotedHeaderFilename("<foo.xyz>"));

  // Negative cases.
  EXPECT_FALSE(IsQuotedHeaderFilename("\"foo.c\""));
  EXPECT_FALSE(IsQuotedHeaderFilename("\"bar/foo.cc\""));
  EXPECT_FALSE(IsQuotedHeaderFilename("<source.cpp>"));
}

}  // namespace
}  // namespace include_what_you_use
