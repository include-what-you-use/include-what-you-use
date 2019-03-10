//===--- iwyu_include_picker_test.cc - test the iwyu include-picker file --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests the internals of iwyu_include_picker.{h,cc}, and a few related
// functions from iwyu_path_util.h

#include "iwyu_include_picker.h"

#include <stddef.h>
#include <stdio.h>
#include <algorithm>
#include <string>
#include <vector>

#include "iwyu_globals.h"
#include "iwyu_path_util.h"
#include "testing/base/public/gunit.h"

namespace clang {
class ASTConsumer;
class ASTFrontendAction;
class CompilerInstance;
}  // namespace clang

using clang::ASTConsumer;
using clang::ASTFrontendAction;
using clang::CompilerInstance;

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

TEST(IsSystemIncludeFile, Basic) {
  EXPECT_FALSE(IsSystemIncludeFile("foo.h"));
  EXPECT_TRUE(IsSystemIncludeFile("/usr/include/string.h"));
  EXPECT_TRUE(IsSystemIncludeFile("/usr/include/c++/4.3/bits/stl_vector.h"));
}

TEST(ConvertToQuotedInclude, Basic) {
  EXPECT_EQ("\"foo.h\"", ConvertToQuotedInclude("foo.h"));
  EXPECT_EQ("<string.h>", ConvertToQuotedInclude("/usr/include/string.h"));
  EXPECT_EQ("<bits/stl_vector.h>",
            ConvertToQuotedInclude("/usr/include/c++/4.3/bits/stl_vector.h"));
}


TEST(DynamicMapping, DoesMapping) {
  IncludePicker p;
  p.AddDirectInclude("project/public/foo.h", "project/internal/private.h", "");
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepath("project/internal/private.h"),
      "\"project/public/foo.h\"");
}

TEST(DynamicMapping, MultiplePublicFiles) {
  IncludePicker p;
  p.AddDirectInclude("project/public/foo.h", "project/internal/private.h", "");
  p.AddDirectInclude("project/public/bar.h", "project/internal/private.h", "");
  p.AddDirectInclude("project/public/bar.h", "project/internal/other.h", "");
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepath("project/internal/private.h"),
      "\"project/public/foo.h\"", "\"project/public/bar.h\"");
}

TEST(DynamicMapping, TransitiveMapping) {
  IncludePicker p;
  p.AddDirectInclude("project/public/foo.h", "project/internal/private.h", "");
  p.AddDirectInclude("project/internal/private.h", "project/internal/other.h",
                     "");
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepath("project/internal/other.h"),
      "\"project/public/foo.h\"");
}

TEST(DynamicMapping, MultipleTransitiveMapping) {
  IncludePicker p;
  p.AddDirectInclude("project/public/foo.h", "project/internal/private.h", "");
  p.AddDirectInclude("project/public/bar.h", "project/internal/private.h", "");
  p.AddDirectInclude("project/public/baz.h", "project/internal/private2.h", "");
  p.AddDirectInclude("project/internal/private.h", "project/internal/other.h",
                     "");
  p.AddDirectInclude("project/internal/private2.h", "project/internal/other.h",
                     "");
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepath("project/internal/other.h"),
      "\"project/public/foo.h\"", "\"project/public/bar.h\"",
      "\"project/public/baz.h\"");
}

TEST(DynamicMapping, NormalizesAsm) {
  IncludePicker p;
  p.AddDirectInclude("/usr/include/types.h",
                     "/usr/include/asm-cris/posix_types.h", "");
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepath("/usr/src/linux-headers-2.6.24-gg23/"
                                       "include/asm-cris/posix_types.h"),
      "<asm/posix_types.h>");
}

TEST(DynamicMapping, PrivateToPublicMapping) {
  IncludePicker p;
  // These names are not the public/internal names that AddInclude looks at.
  p.AddMapping("\"project/private/foo.h\"", "\"project/not_private/bar.h\"");
  p.MarkIncludeAsPrivate("\"project/private/foo.h\"");
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepath("project/private/foo.h"),
      "\"project/not_private/bar.h\"");
}

TEST(GetCandidateHeadersForSymbol, Basic) {
  IncludePicker p;
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(p.GetCandidateHeadersForSymbol("dev_t"),
                      "<sys/types.h>", "<sys/stat.h>");
  EXPECT_VECTOR_STREQ(p.GetCandidateHeadersForSymbol("NULL"),
                      "<stddef.h>", "<cstddef>", "<clocale>", "<cstdio>",
                      "<cstdlib>", "<cstring>", "<ctime>", "<cwchar>",
                      "<locale.h>", "<stdio.h>", "<stdlib.h>", "<string.h>",
                      "<time.h>", "<wchar.h>"
                      );
  EXPECT_VECTOR_STREQ(p.GetCandidateHeadersForSymbol("std::allocator"),
                      "<memory>", "<string>", "<vector>", "<map>", "<set>");
  EXPECT_EQ(0, p.GetCandidateHeadersForSymbol("foo").size());
}

TEST(GetCandidateHeadersForFilepath, C) {
  IncludePicker p;
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepath("/usr/include/bits/dlfcn.h"),
      "<dlfcn.h>");
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepath("/usr/grte/v1/include/assert.h"),
      "<assert.h>", "<cassert>");
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepath("/usr/grte/v1/include/stdarg.h"),
      "<stdarg.h>", "<cstdarg>");
}

TEST(GetCandidateHeadersForFilepath, CXX) {
  IncludePicker p;
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepath("/usr/include/c++/4.2/bits/allocator.h"),
      "<memory>");
}

TEST(GetCandidateHeadersForFilepath, NoIdentityRegex) {
  IncludePicker p;
  // Make sure we don't complain when the key of a mapping is a regex
  // that includes the value (which would, naively, lead to an identity
  // mapping).
  p.AddMapping("@\"mydir/.*\\.h\"", "\"mydir/include.h\"");
  p.MarkIncludeAsPrivate("@\"mydir/.*\\.h\"");   // will *not* apply to include.h!
  // Add a direct include that should be mapped, and that already is.
  p.AddDirectInclude("a.h", "mydir/internal.h", "");
  p.AddDirectInclude("b.h", "mydir/include.h", "");
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepath("mydir/internal.h"),
      "\"mydir/include.h\"");
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepath("mydir/include.h"),
      "\"mydir/include.h\"");
}

TEST(GetCandidateHeadersForFilepath, NotInAnyMap) {
  IncludePicker p;
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepath("/usr/grte/v1/include/poll.h"),
      "<poll.h>");
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepath("././././my/dot.h"),
      "\"my/dot.h\"");
}

TEST(GetCandidateHeadersForFilepath, IncludeRecursion) {
  IncludePicker p;
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepath("/usr/include/c++/4.2/bits/istream.tcc"),
      "<istream>", "<fstream>", "<iostream>", "<sstream>");
}

TEST(GetCandidateHeadersForFilepath, PrivateValueInRecursion) {
  IncludePicker p;
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepath("/usr/include/linux/errno.h"),
      "<errno.h>", "<cerrno>");
}

TEST(GetCandidateHeadersForFilepath, NoBuiltin) {
  // Make sure we never specify "<built-in>" as an #include mapping.
  IncludePicker p;
  p.AddDirectInclude("<built-in>", "foo/bar/internal/code.cc", "");
  p.AddDirectInclude("foo/bar/internal/code.cc", "foo/qux/internal/lib.h", "");

  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepath("foo/qux/internal/lib.h"),
      "\"foo/qux/internal/lib.h\"");
}

TEST(GetCandidateHeadersForFilepathIncludedFrom, NoInternal) {
  IncludePicker p;
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepathIncludedFrom("/usr/include/bits/dlfcn.h",
                                                   "mydir/myapp.h"),
      "<dlfcn.h>");
}

TEST(GetCandidateHeadersForFilepathIncludedFrom, Internal) {
  IncludePicker p;
  // clang always has <built-in> #including the file specified on the cmdline.
  p.AddDirectInclude("<built-in>", "foo/bar/internal/code.cc", "");
  p.AddDirectInclude("foo/bar/internal/code.cc", "foo/bar/public/code.h", "");
  p.AddDirectInclude("foo/bar/public/code.h", "foo/bar/internal/hdr.h", "");
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepathIncludedFrom("foo/bar/internal/hdr.h",
                                                   "foo/bar/internal/code.cc"),
      "\"foo/bar/internal/hdr.h\"");
}

TEST(GetCandidateHeadersForFilepathIncludedFrom, OtherInternal) {
  IncludePicker p;
  p.AddDirectInclude("foo/bar/public/code.h", "foo/bar/internal/hdr.h", "");
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepathIncludedFrom("foo/bar/internal/hdr.h",
                                                   "baz/internal/code.cc"),
      "\"foo/bar/public/code.h\"");
}

TEST(GetCandidateHeadersForFilepathIncludedFrom, PublicToInternal) {
  IncludePicker p;
  p.AddDirectInclude("foo/bar/public/code.cc", "foo/bar/public/code.h", "");
  p.AddDirectInclude("foo/bar/public/code.cc", "foo/bar/public/code2.h", "");
  p.AddDirectInclude("foo/bar/public/code.h", "foo/bar/internal/hdr.h", "");
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepathIncludedFrom("foo/bar/internal/hdr.h",
                                                   "foo/bar/public/code.h"),
      "\"foo/bar/internal/hdr.h\"");
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepathIncludedFrom("foo/bar/internal/hdr.h",
                                                   "foo/bar/public/code2.h"),
      "\"foo/bar/internal/hdr.h\"");
}

TEST(GetCandidateHeadersForFilepathIncludedFrom, FriendRegex) {
  IncludePicker p;
  p.AddDirectInclude("baz.cc", "baz.h", "");
  p.AddDirectInclude("baz.cc", "abcde.h", "");
  p.AddDirectInclude("baz.cc", "random.h", "");
  p.AddDirectInclude("baz.h", "project/private/bar.h", "");
  p.AddDirectInclude("abcde.h", "project/private/bar.h", "");
  p.AddDirectInclude("random.h", "project/private/bar.h", "");
  p.AddMapping("\"project/private/bar.h\"", "\"foo.h\"");
  p.MarkIncludeAsPrivate("\"project/private/bar.h\"");
  p.AddFriendRegex("project/private/bar.h", "\"baz.*\"");
  p.AddFriendRegex("project/private/bar.h", "\"a.c.+\\.h\"");
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepathIncludedFrom("project/private/bar.h",
                                                   "random.h"),
      "\"foo.h\"");
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepathIncludedFrom("project/private/bar.h",
                                                   "baz.h"),
      "\"project/private/bar.h\"");
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepathIncludedFrom("project/private/bar.h",
                                                   "abcde.h"),
      "\"project/private/bar.h\"");
}

TEST(GetCandidateHeadersForFilepathIncludedFrom, PreservesWrittenForm) {
  IncludePicker p;
  p.AddDirectInclude("baz.cc", "baz.h", "\"./././baz.h\"");
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetCandidateHeadersForFilepathIncludedFrom("baz.h", "baz.cc"),
      "\"./././baz.h\"");
}

TEST(HasMapping, IncludeMatch) {
  IncludePicker p;
  p.FinalizeAddedIncludes();
  EXPECT_TRUE(p.HasMapping("/usr/include/stdio.h",
                           "/usr/include/c++/4.2/cstdio"));
  EXPECT_TRUE(p.HasMapping("/usr/include/c++/4.2/bits/stl_deque.h",
                           "/usr/include/c++/4.2/deque"));
  EXPECT_TRUE(p.HasMapping("/usr/include/bits/stat.h",
                           "/usr/include/sys/stat.h"));
  EXPECT_FALSE(p.HasMapping("/usr/include/bits/syscall.h",
                            "/usr/include/sys/stat.h"));
}

TEST(HasMapping, IncludeMatchIndirectly) {
  IncludePicker p;
  p.FinalizeAddedIncludes();
  EXPECT_TRUE(p.HasMapping("/usr/include/c++/4.2/ios",
                           "/usr/include/c++/4.2/iostream"));
  EXPECT_TRUE(p.HasMapping("/usr/include/linux/errno.h",
                           "/usr/include/errno.h"));
}

TEST(HasMapping, IncludeMatchDifferentMaps) {
  IncludePicker p;
  p.FinalizeAddedIncludes();
  // Testing when a google path re-exports a c++ system #include.
  EXPECT_TRUE(p.HasMapping("/usr/include/c++/4.2/ostream", "base/logging.h"));
   // Do some indirect checking too.
  EXPECT_TRUE(p.HasMapping("/usr/include/c++/4.2/ios", "base/logging.h"));
}

}  // namespace
}  // namespace include_what_you_use


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  include_what_you_use::InitGlobalsAndFlagsForTesting();
  return RUN_ALL_TESTS();
}
