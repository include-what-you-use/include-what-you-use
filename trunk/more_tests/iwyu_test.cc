//===--- iwyu_test.cc - test helper functions for include-what-you-use ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests the internals of iwyu*.{h,cc}

#include <algorithm>
#include <set>
#include <string>
#include <utility>  // for make_pair
#include <vector>

#include "iwyu_output.h"
#include "iwyu_path_util.h"
#include "iwyu_stl_util.h"
#include "testing/base/public/gunit.h"
#undef ATTRIBUTE_UNUSED
#include "llvm/Support/raw_ostream.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclTemplate.h"

namespace include_what_you_use {

namespace {

string IntToString(int i) {
  char buf[64];   // big enough for any number
  snprintf(buf, sizeof(buf), "%d", i);
  return buf;
}

// Returns a string representing the first element where actual (a vector),
// and expected (an array) differ, or "" if they're identical.
template <size_t kCount> string VectorDiff(const string (&expected)[kCount],
                                           const vector<string>& actual) {
  for (int i = 0; i < std::min(kCount, actual.size()); ++i) {
    if (expected[i] != actual[i]) {
      return ("Differ at #" + IntToString(i) + ": expected=" + expected[i] +
              ", actual=" + actual[i]);
    }
  }
  if (kCount < actual.size()) {
    return ("Differ at #" + IntToString(kCount) +
            ": expected at EOF, actual=" + actual[kCount]);
  } else if (actual.size() < kCount) {
    return ("Differ at #" + IntToString(kCount) + ": expected=" +
            expected[actual.size()] + ", actual at EOF");
  } else {
    return "";
  }
}

// Different format than usual due to VA_ARGS.  First comes the actual
// value (as a string vector), then the expected values (as strings).
#define EXPECT_VECTOR_STREQ(actual, ...)  do {  \
  const string expected[] = { __VA_ARGS__ };    \
  EXPECT_EQ("", VectorDiff(expected, actual));  \
} while (0)


TEST(GetCanonicalName, StripsKnownSuffixes) {
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

TEST(GetCanonicalName, StripsQuotes) {
  EXPECT_EQ("set", GetCanonicalName("<set>"));
  EXPECT_EQ("bits/stl_set", GetCanonicalName("<bits/stl_set.h>"));
  EXPECT_EQ("my/path/foo", GetCanonicalName("\"my/path/foo-inl.h\""));
}

TEST(GetCanonicalName, MapsInternalToPublic) {
  EXPECT_EQ("my/public/foo", GetCanonicalName("my/internal/foo.cc"));
  EXPECT_EQ("my/public/foo", GetCanonicalName("my/public/foo.cc"));
  EXPECT_EQ("my/public/foo", GetCanonicalName("my/internal/foo.h"));
  EXPECT_EQ("my/public/foo", GetCanonicalName("my/public/foo.h"));
  EXPECT_EQ("internal/foo", GetCanonicalName("internal/foo"));
  EXPECT_EQ("path/internal_impl", GetCanonicalName("path/internal_impl.cc"));
}

TEST(IncludePicker, StripPathPrefix) {
  IncludePicker p;
  EXPECT_EQ("foo.h",
            p.StripPathPrefix("foo.h"));
  EXPECT_EQ("third_party/ICU/io.h",
            p.StripPathPrefix("third_party/ICU/io.h"));
  EXPECT_EQ("string.h", p.StripPathPrefix("/usr/include/string.h"));
  EXPECT_EQ("bits/stl_vector.h",
            p.StripPathPrefix("/usr/include/c++/4.3/bits/stl_vector.h"));
}

TEST(IncludePicker, IsSystemIncludeFile) {
  IncludePicker p;
  EXPECT_FALSE(p.IsSystemIncludeFile("foo.h"));
  EXPECT_FALSE(p.IsSystemIncludeFile("third_party/ICU/icu.h"));
  EXPECT_TRUE(p.IsSystemIncludeFile("/usr/include/string.h"));
  EXPECT_TRUE(p.IsSystemIncludeFile("/usr/include/c++/4.3/bits/stl_vector.h"));
}

TEST(IncludePicker, GetQuotedIncludeFor) {
  IncludePicker p;
  EXPECT_EQ("\"foo.h\"", p.GetQuotedIncludeFor("foo.h"));
  EXPECT_EQ("\"third_party/ICU/icu.h\"",
            p.GetQuotedIncludeFor("third_party/ICU/icu.h"));
  EXPECT_EQ("<string.h>", p.GetQuotedIncludeFor("/usr/include/string.h"));
  EXPECT_EQ("<bits/stl_vector.h>",
            p.GetQuotedIncludeFor("/usr/include/c++/4.3/bits/stl_vector.h"));
}

TEST(IncludePicker, GetQuotedIncludeFor_NormalizesAsm) {
  IncludePicker p;
  EXPECT_EQ("<asm/posix_types.h>",
            p.GetQuotedIncludeFor("/usr/src/linux-headers-2.6.24-gg23/"
                                  "include/asm-cris/posix_types.h"));
  // This isn't an asm, so should be left alone.
  EXPECT_EQ("<linux/ioctl.h>",
            p.GetQuotedIncludeFor("/usr/src/linux-headers-2.6.24-gg23/"
                                  "include/linux/ioctl.h"));
}

TEST(IncludePicker, DynamicMapping_DoesMapping) {
  IncludePicker p;
  p.AddDirectInclude("project/public/foo.h", "\"project/internal/private.h\"");
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForPrivateHeader("project/internal/private.h"),
      "\"project/public/foo.h\"");
}

TEST(IncludePicker, DynamicMapping_MultiplePublicFiles) {
  IncludePicker p;
  p.AddDirectInclude("project/public/foo.h", "\"project/internal/private.h\"");
  p.AddDirectInclude("project/public/bar.h", "\"project/internal/private.h\"");
  p.AddDirectInclude("project/public/bar.h", "\"project/internal/other.h\"");
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForPrivateHeader("project/internal/private.h"),
      "\"project/public/foo.h\"", "\"project/public/bar.h\"");
}

TEST(IncludePicker, DynamicMapping_TransitiveMapping) {
  IncludePicker p;
  p.AddDirectInclude("project/public/foo.h", "\"project/internal/private.h\"");
  p.AddDirectInclude("project/internal/private.h",
                     "\"project/internal/other.h\"");
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForPrivateHeader("project/internal/other.h"),
      "\"project/public/foo.h\"");
}

TEST(IncludePicker, DynamicMapping_MultipleTransitiveMapping) {
  IncludePicker p;
  p.AddDirectInclude("project/public/foo.h", "\"project/internal/private.h\"");
  p.AddDirectInclude("project/public/bar.h", "\"project/internal/private.h\"");
  p.AddDirectInclude("project/public/baz.h", "\"project/internal/private2.h\"");
  p.AddDirectInclude("project/internal/private.h",
                     "\"project/internal/other.h\"");
  p.AddDirectInclude("project/internal/private2.h",
                     "\"project/internal/other.h\"");
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForPrivateHeader("project/internal/other.h"),
      "\"project/public/foo.h\"", "\"project/public/bar.h\"",
      "\"project/public/baz.h\"");
}

TEST(IncludePicker, DynamicMapping_PrivateToPublicMapping) {
  IncludePicker p;
  // These names are not the public/internal names that AddInclude looks at.
  p.AddPrivateToPublicMapping("\"project/private/foo.h",
                              "project/not_private/bar.h");
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForPrivateHeader("project/private/foo.h"),
      "\"project/not_private/bar.h\"");
}

TEST(IncludePicker, GetPublicHeadersForSymbol) {
  IncludePicker p;
  EXPECT_VECTOR_STREQ(p.GetPublicHeadersForSymbol("dev_t"),
                      "<sys/types.h>", "<sys/stat.h>");
  EXPECT_VECTOR_STREQ(p.GetPublicHeadersForSymbol("NULL"),
                      "<stddef.h>", "<clocale>", "<cstddef>", "<cstdio>",
                      "<cstdlib>", "<cstring>", "<ctime>", "<cwchar>",
                      "<locale.h>", "<stdio.h>", "<stdlib.h>", "<string.h>",
                      "<time.h>", "<wchar.h>");
  EXPECT_VECTOR_STREQ(p.GetPublicHeadersForSymbol("std::ios"), "<ios>");
  EXPECT_EQ(0, p.GetPublicHeadersForSymbol("foo").size());
}

TEST(IncludePicker, GetPublicHeadersForPrivateHeader_C) {
  IncludePicker p;
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForPrivateHeader("/usr/include/bits/dlfcn.h"),
      "<dlfcn.h>");
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForPrivateHeader("third_party/grte/v2/release/include/"
                                         "bits/mathcalls.h"),
      "<math.h>", "<cmath>");
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForPrivateHeader("/usr/grte/v1/include/assert.h"),
      "<assert.h>", "<cassert>");
}

TEST(IncludePicker, GetPublicHeadersForPrivateHeader_CXX) {
  IncludePicker p;
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForPrivateHeader("/usr/include/c++/4.2/"
                                         "bits/allocator.h"),
      "<memory>");
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForPrivateHeader(
          "third_party/llvm/crosstool/gcc-4.4.0-glibc-2.3.6-grte/x86/"
          "include/c++/4.4.0/backward/hash_fun.h"),
      "<hash_map>", "<hash_set>");
}

TEST(IncludePicker, GetPublicHeadersForPrivateHeader_ThirdParty) {
  IncludePicker p;
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForPrivateHeader("third_party/dynamic_annotations/d.h"),
      "\"base/dynamic_annotations.h\"");
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForPrivateHeader("third_party/dynamic_annotations/"
                                         "a/b/c.h"),
      "\"base/dynamic_annotations.h\"");
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForPrivateHeader("third_party/python2_4_3/includes/"
                                         "py.h"),
      "<Python.h>");
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForPrivateHeader("third_party/icu/include/unicode/"
                                         "udraft.h"),
      "\"third_party/icu/include/unicode/utypes.h\"");
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForPrivateHeader("third_party/icu/include/unicode/"
                                         "ukeep.h"),
      "\"third_party/icu/include/unicode/ukeep.h\"");
}

TEST(IncludePicker, GetPublicHeadersForPrivateHeader_NotInAnyMap) {
  IncludePicker p;
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForPrivateHeader("/usr/grte/v1/include/poll.h"),
      "<poll.h>");
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForPrivateHeader("third_party/llvm/crosstool/"
                                         "gcc-4.4.0-glibc-2.3.6-grte/x86/"
                                         "include/c++/4.4.0/vector"),
      "<vector>");
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForPrivateHeader("././././my/dot.h"),
      "\"my/dot.h\"");
}

TEST(IncludePicker, GetPublicHeadersForPrivateHeader_IncludeRecursion) {
  IncludePicker p;
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForPrivateHeader("/usr/include/c++/4.2/"
                                         "bits/istream.tcc"),
      "<istream>", "<fstream>", "<iostream>", "<sstream>");
}

TEST(IncludePicker, GetPublicHeadersForPrivateHeader_PrivateValueInRecursion) {
  IncludePicker p;
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForPrivateHeader("/usr/include/linux/errno.h"),
      "<errno.h>", "<cerrno>");
}

TEST(IncludePicker, PathReexportsIncludeMatch) {
  IncludePicker p;
  EXPECT_TRUE(p.PathReexportsInclude("/usr/include/c++/4.2/cstdio",
                                     "/usr/include/stdio.h"));
  EXPECT_TRUE(p.PathReexportsInclude("/usr/include/c++/4.2/deque",
                                     "/usr/include/c++/4.2/bits/stl_deque.h"));
  EXPECT_TRUE(p.PathReexportsInclude("/usr/include/sys/stat.h",
                                     "/usr/include/bits/stat.h"));
  EXPECT_FALSE(p.PathReexportsInclude("/usr/include/sys/stat.h",
                                      "/usr/include/bits/syscall.h"));
}

TEST(IncludePicker, PathReexportsIncludeMatchIndirectly) {
  IncludePicker p;
  EXPECT_TRUE(p.PathReexportsInclude("/usr/include/c++/4.2/iostream",
                                     "/usr/include/c++/4.2/ios"));
  EXPECT_TRUE(p.PathReexportsInclude("/usr/include/errno.h",
                                     "/usr/include/linux/errno.h"));
}

TEST(IncludePicker, PathReexportsIncludeMatchDifferentMaps) {
  IncludePicker p;
  // Testing when a google path re-exports a c++ system #include.
  EXPECT_TRUE(p.PathReexportsInclude("base/logging.h",
                                     "/usr/include/c++/4.2/ostream"));
   // Do some indirect checking too.
  EXPECT_TRUE(p.PathReexportsInclude("base/logging.h",
                                     "/usr/include/c++/4.2/ios"));
}

TEST(IncludePicker, PathReexportsIncludeForThirdParty) {
  IncludePicker p;
  EXPECT_TRUE(p.PathReexportsInclude(
      "base/dynamic_annotations.h",
      "third_party/dynamic_annotations/foo/bar.h"));
}

}  // namespace
}  // namespace include_what_you_use
