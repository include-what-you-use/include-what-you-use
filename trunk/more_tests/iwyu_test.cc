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

#include "iwyu_driver.h"
#include "iwyu_globals.h"
#include "iwyu_output.h"
#include "iwyu_path_util.h"
#include "iwyu_stl_util.h"
#include "testing/base/public/gunit.h"
#undef ATTRIBUTE_UNUSED
#include "llvm/Support/raw_ostream.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"

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

TEST(IncludePicker, IsSystemIncludeFile) {
  EXPECT_FALSE(IsSystemIncludeFile("foo.h"));
  EXPECT_FALSE(IsSystemIncludeFile("third_party/ICU/icu.h"));
  EXPECT_TRUE(IsSystemIncludeFile("/usr/include/string.h"));
  EXPECT_TRUE(IsSystemIncludeFile("/usr/include/c++/4.3/bits/stl_vector.h"));
}

TEST(IncludePicker, ConvertToQuotedInclude) {
  EXPECT_EQ("\"foo.h\"", ConvertToQuotedInclude("foo.h"));
  EXPECT_EQ("\"third_party/ICU/icu.h\"",
            ConvertToQuotedInclude("third_party/ICU/icu.h"));
  EXPECT_EQ("<string.h>", ConvertToQuotedInclude("/usr/include/string.h"));
  EXPECT_EQ("<bits/stl_vector.h>",
            ConvertToQuotedInclude("/usr/include/c++/4.3/bits/stl_vector.h"));
}

TEST(IncludePicker, GetQuotedIncludeFor_NormalizesAsm) {
  EXPECT_EQ("<asm/posix_types.h>",
            ConvertToQuotedInclude("/usr/src/linux-headers-2.6.24-gg23/"
                                   "include/asm-cris/posix_types.h"));
  // This isn't an asm, so should be left alone.
  EXPECT_EQ("<linux/ioctl.h>",
            ConvertToQuotedInclude("/usr/src/linux-headers-2.6.24-gg23/"
                                   "include/linux/ioctl.h"));
}

TEST(IncludePicker, DynamicMapping_DoesMapping) {
  IncludePicker p;
  p.AddDirectInclude("project/public/foo.h", "\"project/internal/private.h\"");
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForFilepath("project/internal/private.h"),
      "\"project/public/foo.h\"");
}

TEST(IncludePicker, DynamicMapping_MultiplePublicFiles) {
  IncludePicker p;
  p.AddDirectInclude("project/public/foo.h", "\"project/internal/private.h\"");
  p.AddDirectInclude("project/public/bar.h", "\"project/internal/private.h\"");
  p.AddDirectInclude("project/public/bar.h", "\"project/internal/other.h\"");
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForFilepath("project/internal/private.h"),
      "\"project/public/foo.h\"", "\"project/public/bar.h\"");
}

TEST(IncludePicker, DynamicMapping_TransitiveMapping) {
  IncludePicker p;
  p.AddDirectInclude("project/public/foo.h", "\"project/internal/private.h\"");
  p.AddDirectInclude("project/internal/private.h",
                     "\"project/internal/other.h\"");
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForFilepath("project/internal/other.h"),
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
      p.GetPublicHeadersForFilepath("project/internal/other.h"),
      "\"project/public/foo.h\"", "\"project/public/bar.h\"",
      "\"project/public/baz.h\"");
}

TEST(IncludePicker, DynamicMapping_PrivateToPublicMapping) {
  IncludePicker p;
  // These names are not the public/internal names that AddInclude looks at.
  p.AddMapping("\"project/private/foo.h\"", "\"project/not_private/bar.h\"");
  p.MarkIncludeAsPrivate("\"project/private/foo.h\"");
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForFilepath("project/private/foo.h"),
      "\"project/not_private/bar.h\"");
}

TEST(IncludePicker, GetPublicHeadersForSymbol) {
  IncludePicker p;
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(p.GetPublicHeadersForSymbol("dev_t"),
                      "<sys/types.h>", "<sys/stat.h>");
  EXPECT_VECTOR_STREQ(p.GetPublicHeadersForSymbol("NULL"),
                      "<stddef.h>", "<clocale>", "<cstddef>", "<cstdio>",
                      "<cstdlib>", "<cstring>", "<ctime>", "<cwchar>",
                      "<locale.h>", "<stdio.h>", "<stdlib.h>", "<string.h>",
                      "<time.h>", "<wchar.h>");
  EXPECT_VECTOR_STREQ(p.GetPublicHeadersForSymbol("std::ios"),
                      "<ios>", "<istream>", "<fstream>", "<iostream>",
                      "<sstream>", "<ostream>", "\"base/logging.h\"");
  EXPECT_EQ(0, p.GetPublicHeadersForSymbol("foo").size());
}

TEST(IncludePicker, GetPublicHeadersForFilepath_C) {
  IncludePicker p;
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForFilepath("/usr/include/bits/dlfcn.h"),
      "<dlfcn.h>");
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForFilepath("third_party/grte/v2/release/include/"
                                    "bits/mathcalls.h"),
      "<math.h>", "<cmath>");
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForFilepath("/usr/grte/v1/include/assert.h"),
      "<assert.h>", "<cassert>");
}

TEST(IncludePicker, GetPublicHeadersForFilepath_CXX) {
  IncludePicker p;
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForFilepath("/usr/include/c++/4.2/bits/allocator.h"),
      "<memory>");
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForFilepath(
          "third_party/llvm/crosstool/gcc-4.4.0-glibc-2.3.6-grte/x86/"
          "include/c++/4.4.0/backward/hash_fun.h"),
      "<hash_map>", "<hash_set>");
}

TEST(IncludePicker, GetPublicHeadersForFilepath_ThirdParty) {
  IncludePicker p;
  // For globs to work, we need to have actually seen the includes.
  p.AddDirectInclude("\"a.h\"", "\"third_party/dynamic_annotations/d.h\"");
  p.AddDirectInclude("\"b.h\"", "\"third_party/dynamic_annotations/a/b/c.h\"");
  p.AddDirectInclude("\"c.h\"", "\"third_party/python2_4_3/includes/py.h\"");
  p.AddDirectInclude("\"d.h\"", "\"third_party/isu/include/unicode/udraft.h\"");
  p.AddDirectInclude("\"e.h\"", "\"third_party/isu/include/unicode/ukeep.h\"");
  p.FinalizeAddedIncludes();

  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForFilepath("third_party/dynamic_annotations/d.h"),
      "\"base/dynamic_annotations.h\"");
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForFilepath("third_party/dynamic_annotations/a/b/c.h"),
      "\"base/dynamic_annotations.h\"");
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForFilepath("third_party/python2_4_3/includes/py.h"),
      "<Python.h>");
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForFilepath("third_party/icu/include/unicode/udraft.h"),
      "\"third_party/icu/include/unicode/utypes.h\"");
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForFilepath("third_party/icu/include/unicode/ukeep.h"),
      "\"third_party/icu/include/unicode/ukeep.h\"");
}

TEST(IncludePicker, GetPublicHeadersForFilepath_GlobOverlap) {
  IncludePicker p;
  // It's ok if a header is specified in both a glob and non-glob rule.
  // For globs to work, we need to have actually seen the includes.
  p.AddDirectInclude("\"a.h\"", "\"third_party/dynamic_annotations/d.h\"");
  p.AddMapping("\"third_party/dynamic_annotations/d.h\"",
               "\"third_party/dynamic_annotations/public.h\"");
  p.MarkIncludeAsPrivate("\"third_party/dynamic_annotations/d.h\"");
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForFilepath("third_party/dynamic_annotations/d.h"),
      "\"third_party/dynamic_annotations/public.h\"",
      "\"base/dynamic_annotations.h\"");
}

TEST(IncludePicker, GetPublicHeadersForFilepath_NotInAnyMap) {
  IncludePicker p;
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForFilepath("/usr/grte/v1/include/poll.h"),
      "<poll.h>");
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForFilepath("third_party/llvm/crosstool/"
                                    "gcc-4.4.0-glibc-2.3.6-grte/x86/"
                                    "include/c++/4.4.0/vector"),
      "<vector>");
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForFilepath("././././my/dot.h"),
      "\"my/dot.h\"");
}

TEST(IncludePicker, GetPublicHeadersForFilepath_IncludeRecursion) {
  IncludePicker p;
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForFilepath("/usr/include/c++/4.2/bits/istream.tcc"),
      "<istream>", "<fstream>", "<iostream>", "<sstream>");
}

TEST(IncludePicker, GetPublicHeadersForFilepath_PrivateValueInRecursion) {
  IncludePicker p;
  p.FinalizeAddedIncludes();
  EXPECT_VECTOR_STREQ(
      p.GetPublicHeadersForFilepath("/usr/include/linux/errno.h"),
      "<errno.h>", "<cerrno>");
}

TEST(IncludePicker, HasMappingIncludeMatch) {
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

TEST(IncludePicker, HasMappingIncludeMatchIndirectly) {
  IncludePicker p;
  p.FinalizeAddedIncludes();
  EXPECT_TRUE(p.HasMapping("/usr/include/c++/4.2/ios",
                           "/usr/include/c++/4.2/iostream"));
  EXPECT_TRUE(p.HasMapping("/usr/include/linux/errno.h",
                           "/usr/include/errno.h"));
}

TEST(IncludePicker, HasMappingIncludeMatchDifferentMaps) {
  IncludePicker p;
  p.FinalizeAddedIncludes();
  // Testing when a google path re-exports a c++ system #include.
  EXPECT_TRUE(p.HasMapping("/usr/include/c++/4.2/ostream", "base/logging.h"));
   // Do some indirect checking too.
  EXPECT_TRUE(p.HasMapping("/usr/include/c++/4.2/ios", "base/logging.h"));
}

TEST(IncludePicker, HasMappingIncludeForThirdParty) {
  IncludePicker p;
  // For globs to work, we need to have actually seen the includes.
  p.AddDirectInclude("\"base/dynamic_annotations.h\"",
                     "\"third_party/dynamic_annotations/foo/bar.h\"");
  p.FinalizeAddedIncludes();
  EXPECT_TRUE(p.HasMapping("third_party/dynamic_annotations/foo/bar.h",
                           "base/dynamic_annotations.h"));
}

class InitGlobalsAction : public ASTFrontendAction {
  virtual ASTConsumer* CreateASTConsumer(CompilerInstance& compiler,  // NOLINT
                                         llvm::StringRef /* dummy */) {
    InitGlobals(&compiler.getSourceManager());
    return NULL;
  }
};

}  // namespace
}  // namespace include_what_you_use


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  // We make a fake compile command.  We only need this for the header
  // search info, so it doesn't matter exactly what it says we're compiling.
  // We are responsible for freeing this, but we're in main, so who cares?
  const char* fake_argv[] = { "iwyu_test", "iwyu_test.cc" };
  const int fake_argc = sizeof(fake_argv) / sizeof(*fake_argv);
  CompilerInstance* compiler = CreateCompilerInstance(fake_argc, fake_argv);
  CHECK_(compiler && "Failed to process fake argv");

  // Create and execute the frontend to generate an LLVM bitcode module.
  include_what_you_use::InitGlobalsAction action;
  if (!compiler->ExecuteAction(action))
    return 1;

  return RUN_ALL_TESTS();
}
