//===--- iwyu_output_test.cpp - test iwyu_output.{h,cc} -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests iwyu_output.{h,cc}

#include <set>
#include "iwyu_ast_util.h"
#include "iwyu_location_util.h"
#include "iwyu_output.h"
#include "iwyu_stl_util.h"
#include "testing/base/public/gunit.h"
#undef ATTRIBUTE_UNUSED
#include "llvm/Support/raw_ostream.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/Basic/SourceLocation.h"

using clang::NamedDecl;
using clang::SourceLocation;

namespace include_what_you_use {

namespace internal {
// Internal routines from iwyu_output.cc exposed for testing purposes.

string MungedForwardDeclareLine(const NamedDecl* decl);

set<string> CalculateMinimalIncludes(
    const set<string>& direct_includes,
    const set<string>& associated_direct_includes,
    vector<OneUse>* uses);

void ProcessForwardDeclare(OneUse* use);

void ProcessFullUse(OneUse* use);

void ProcessSymbolUse(OneUse* use);

void CalculateIwyuForForwardDeclareUse(OneUse* use,
                                       const set<string>& actual_includes,
                                       const set<string>& desired_includes);

void CalculateIwyuForFullUse(OneUse* use,
                             const set<string>& actual_includes,
                             const set<string>& desired_includes);

string SanitizeSymbol(string name);

void CalculateDesiredIncludesAndForwardDeclares(
    const vector<OneUse>& uses,
    const set<string> associated_desired_includes,
    vector<OneIncludeOrForwardDeclareLine>* lines);

string PrintableIncludeOrForwardDeclareLine(
    const OneIncludeOrForwardDeclareLine& line,
    const set<string>& associated_quoted_includes);

string PrintableDiffs(const string& filename,
                      const set<string>& associated_quoted_includes,
                      const vector<OneIncludeOrForwardDeclareLine>& lines);

struct FakeSourceLocation : public SourceLocation {
  FakeSourceLocation(const string& fp, int ln)
      : SourceLocation(), filepath(fp), linenum(ln) { }
  string filepath;
  int linenum;
};

}  // namespace internal

using internal::FakeNamedDecl;   // from iwyu_output.h
using internal::FakeSourceLocation;


// Some helper routines on fakes.
// TODO(csilvers): these don't get called where we want them to. :-(
string GetFilePath(const FakeSourceLocation& fake_loc) {
  return fake_loc.filepath;
}

string GetFilePath(const FakeNamedDecl* fake_decl) {
  return fake_decl->decl_filepath();
}

// Note these return a string, not a FileEntry*.
string GetFileEntry(const FakeSourceLocation& fake_loc) {
  return fake_loc.filepath;
}

string GetFileEntry(const FakeNamedDecl* fake_decl) {
  return fake_decl->decl_filepath();
}

// DynCasting a fake always fails.
inline internal::DynCastPtr<FakeNamedDecl> DynCastFrom(FakeNamedDecl* ptr) {
  return internal::DynCastPtr<FakeNamedDecl>(NULL);
}

namespace {

// A helper for creating an IncludeLineData object.
template <int N>
OneIncludeOrForwardDeclareLine MakeDesiredIncludeLine(
    const string& quoted_includee, const char* (&used_symbols)[N]) {
  OneIncludeOrForwardDeclareLine retval(quoted_includee, 1);
  retval.set_desired();
  for (int i = 0; i < N; i++)
    retval.AddSymbolUse(used_symbols[i]);
  return retval;
}

// Common case where we only need one symbol.
OneIncludeOrForwardDeclareLine MakeDesiredIncludeLine(
    const string& quoted_includee, const char* used_symbol) {
  const char* used_symbols[1] = { used_symbol };
  return MakeDesiredIncludeLine(quoted_includee, used_symbols);
}

TEST(MungedForwardDeclareLineTest, Works) {
  // TODO(csilvers): figure out how to test on a real RecordDecl/etc.
}

TEST(CalculateMinimalIncludes, Works) {
  // TODO(csilvers): add tests here
}

TEST(ProcessForwardDeclareTest, A1) {
  // TODO(csilvers): figure out how to test with a real record_decl/tpl_decl
}

TEST(ProcessForwardDeclareTest, A2) {
  // TODO(csilvers): figure out how to test with a real class_tpl_decl
}

TEST(ProcessForwardDeclareTest, A3) {
  // TODO(csilvers): figure out how to test with a real record_decl
}

TEST(ProcessForwardDeclareTest, A4) {
  // TODO(csilvers): figure out how to test with a real record_decl
}

TEST(ProcessFullUseTest, B1) {
  return;  // TODO(csilvers): re-enable when we can fake the SourceLoc and decl
  FakeNamedDecl decl("class", "MyClass", "src/includes/myclass.h", 10);
  // Test the use being *before* the definition in the file (shouldn't matter)
  OneUse samefile_use(&decl, FakeSourceLocation("src/includes/myclass.h", 5),
                      OneUse::kFullUse);
  OneUse difffile_use(&decl, FakeSourceLocation("src/myclass.cc", 10),
                      OneUse::kFullUse);
  internal::ProcessFullUse(&samefile_use);
  internal::ProcessFullUse(&difffile_use);
  EXPECT_TRUE(samefile_use.ignore_use());
  EXPECT_FALSE(difffile_use.ignore_use());
}

TEST(ProcessFullUseTest, B2) {
  // TODO(csilvers): figure out how to test with a real fn_decl
}

TEST(ProcessFullUseTest, B3) {
  return;  // TODO(csilvers): re-enable when we can fake the SourceLoc and decl
  FakeNamedDecl cc_decl("class", "MyClass", "src/myclass.cc", 10);
  // Test the use being *before* the definition in the file (shouldn't matter)
  OneUse h_use(&cc_decl, FakeSourceLocation("src/includes/myclass.h", 5),
               OneUse::kFullUse);
  OneUse cc_use(&cc_decl, FakeSourceLocation("src/main.cc", 10),
                OneUse::kFullUse);
  internal::ProcessFullUse(&h_use);
  internal::ProcessFullUse(&cc_use);
  EXPECT_TRUE(h_use.ignore_use());
  EXPECT_FALSE(cc_use.ignore_use());
}

TEST(ProcessFullUseTest, B4) {
  // TODO(csilvers): figure out how to test with a real cxx_method_decl
}

TEST(ProcessSymbolUseTest, B5) {
  return;  // TODO(csilvers): re-enable when we can fake the SourceLoc and decl
  // Test the use being *before* the definition in the file (shouldn't matter)
  OneUse samefile_use("mysym", "src/includes/myclass.h",
                      FakeSourceLocation("src/includes/myclass.h", 5));
  OneUse difffile_use("sym2", "src/includes/myclass.h",
                      FakeSourceLocation("src/myclass.cc", 5));
  internal::ProcessSymbolUse(&samefile_use);
  internal::ProcessSymbolUse(&difffile_use);
  EXPECT_TRUE(samefile_use.ignore_use());
  EXPECT_FALSE(difffile_use.ignore_use());
}

TEST(CalculateIwyuForForwardDeclareUseTest, D1) {
  // TODO(csilvers): figure out how to test with a real record_decl
}

TEST(CalculateIwyuForForwardDeclareUseTest, D2) {
  // TODO(csilvers): figure out how to test with a real record_decl
}

TEST(CalculateIwyuForFullUse, E1) {
  OneUse iwyu_violation("mysym", "src/includes/myclass.h",
                        FakeSourceLocation("src/myclass.cc", 5));
  OneUse iwyu_ok("mysym", "src/includes/myclass.h",
                 FakeSourceLocation("src/myclass.cc", 5));
  iwyu_violation.set_suggested_header("<myclass.h>");
  iwyu_ok.set_suggested_header("\"includes/myclass.h\"");

  set<string> actual_includes;
  actual_includes.insert("\"includes/myclass.h\"");
  actual_includes.insert("<stdio.h>");
  const set<string> empty;   // desired includes: doesn't matter for this test

  internal::CalculateIwyuForFullUse(&iwyu_violation, actual_includes, empty);
  internal::CalculateIwyuForFullUse(&iwyu_ok, actual_includes, empty);
  EXPECT_TRUE(iwyu_violation.is_iwyu_violation());
  EXPECT_FALSE(iwyu_ok.is_iwyu_violation());
}

TEST(SanitizeSymbolTest, Works) {
  // Simple name with no template arguments.
  EXPECT_EQ("int", internal::SanitizeSymbol("int"));

  // Name with one template argument.
  EXPECT_EQ("Foo", internal::SanitizeSymbol("Foo<int>"));

  // Name with multiple template arguments.
  EXPECT_EQ("map", internal::SanitizeSymbol("map<int, string>"));

  // Name with nested template arguments.
  EXPECT_EQ("basic_string",
            internal::SanitizeSymbol("basic_string<char, allocator<char> >"));

  // Type with more than one pair of out-most <>.
  EXPECT_EQ("int(Foo, Bar)",
            internal::SanitizeSymbol("int(Foo<bool>, Bar<double>)"));
}

TEST(CalculateDesiredIncludesAndForwardDeclaresTest, Works) {
  // TODO(csilvers): implement
}

// This test fixture automatically saves/restores the verbose level
// between tests to prevent leaking side effects.
class VerboseTest : public ::testing::Test {
 protected:
  VerboseTest() : old_verbose_level_(GetVerboseLevel()) {
    // 2 is the default verbose level for tests that don't care much
    // about the level.
    SetVerboseLevel(2);
  }

  ~VerboseTest() { SetVerboseLevel(old_verbose_level_); }

 private:
  const int old_verbose_level_;
};

class PrintableIncludeOrForwardDeclareLineTest : public VerboseTest { };

TEST_F(PrintableIncludeOrForwardDeclareLineTest, CutsOffAt80Columns) {
  const set<string> empty;
  OneIncludeOrForwardDeclareLine foo_line("<foo.h>", 1);
  foo_line.AddSymbolUse("FOO");
  foo_line.AddSymbolUse("FooClass");
  foo_line.AddSymbolUse("FooClass::a");
  foo_line.AddSymbolUse("FooClass::~FooClass");
  foo_line.AddSymbolUse("FunctionOnFoo");
  foo_line.AddSymbolUse("Foo_Enum");
  foo_line.AddSymbolUse("Foo_Typedef");
  foo_line.set_desired();
  EXPECT_EQ("#include <foo.h>"
            "                        // for FOO, FooClass, FooClass::a, etc\n",
            internal::PrintableIncludeOrForwardDeclareLine(foo_line, empty));

  OneIncludeOrForwardDeclareLine bar_line("<bar.h>", 2);
  bar_line.AddSymbolUse("this_symbol_is_so_big_there_is_no_room_for_it_at_all");
  bar_line.set_desired();
  EXPECT_EQ("#include <bar.h>\n",
            internal::PrintableIncludeOrForwardDeclareLine(bar_line, empty));

  OneIncludeOrForwardDeclareLine long_line(
      "\"deep/enough/to/go/past/the/indent.h\"", 3);
  long_line.AddSymbolUse("DeepFn");
  long_line.AddSymbolUse("Deep_Typedef");
  long_line.AddSymbolUse("DEEP");
  long_line.AddSymbolUse("TheInkyDeeps");
  long_line.set_desired();

  EXPECT_EQ("#include \"deep/enough/to/go/past/the/indent.h\""
            "  // for DEEP, DeepFn, etc\n",
            internal::PrintableIncludeOrForwardDeclareLine(long_line, empty));
}

TEST_F(PrintableIncludeOrForwardDeclareLineTest, SortsSymbolsByFreqThenAlpha) {
  SetVerboseLevel(3);   // so we see a printout of *all* the symbols
  const set<string> empty;
  OneIncludeOrForwardDeclareLine line("<foo.h>", 2);
  line.AddSymbolUse("FOO");
  line.AddSymbolUse("FooClass");
  line.AddSymbolUse("FooClass::a");
  line.AddSymbolUse("Foo_Enum");
  line.AddSymbolUse("FooClass::~FooClass");
  line.AddSymbolUse("FunctionOnFoo");
  line.AddSymbolUse("FooClass");
  line.AddSymbolUse("Foo_Typedef");
  line.AddSymbolUse("FooClass");
  line.AddSymbolUse("Foo_Typedef");
  line.AddSymbolUse("FunctionOnFoo");
  line.set_desired();
  EXPECT_EQ("#include <foo.h>"
            "                        // for FooClass, Foo_Typedef,"
            " FunctionOnFoo, FOO, FooClass::a, FooClass::~FooClass, Foo_Enum\n",
            internal::PrintableIncludeOrForwardDeclareLine(line, empty));
}

TEST(PrintableDiffsTest, PrintsEmptyIncludes) {
  vector<OneIncludeOrForwardDeclareLine> no_lines;
  EXPECT_EQ("\n"
            "(baz.cc has correct #includes/fwd-decls)\n",
            internal::PrintableDiffs("baz.cc", set<string>(), no_lines));
}

TEST(PrintableDiffsTest, ProperlyOrdersIncludeLines) {
  set<string> associated_includes;
  associated_includes.insert("\"baz.h\"");
  associated_includes.insert("\"baz-inl.h\"");
  vector<OneIncludeOrForwardDeclareLine> lines;
  lines.push_back(MakeDesiredIncludeLine("\"foo.h\"", "FOO"));
  lines.push_back(MakeDesiredIncludeLine("\"bar.h\"", "BAR"));
  lines.push_back(MakeDesiredIncludeLine("\"baz.h\"", "BAZ"));
  lines.push_back(MakeDesiredIncludeLine("\"foo-inl.h\"", "FooFn"));
  lines.push_back(MakeDesiredIncludeLine("\"bar-inl.h\"", "BarFn"));
  lines.push_back(MakeDesiredIncludeLine("\"baz-inl.h\"", "BazFn"));
  lines.push_back(MakeDesiredIncludeLine("<stdio.h>", "printf"));
  lines.push_back(MakeDesiredIncludeLine("<string>", "string::iterator"));
  lines.push_back(MakeDesiredIncludeLine("<sstring>", "sstring"));
  lines.push_back(MakeDesiredIncludeLine("<ctype.h>", "isascii"));
  EXPECT_EQ("\n"
            "baz.cc should add these lines:\n"
            "#include \"baz.h\"\n"
            "#include \"baz-inl.h\"\n"
            "#include <ctype.h>                      // for isascii\n"
            "#include <stdio.h>                      // for printf\n"
            "#include <sstring>                      // for sstring\n"
            "#include <string>                       // for string::iterator\n"
            "#include \"bar-inl.h\"                    // for BarFn\n"
            "#include \"bar.h\"                        // for BAR\n"
            "#include \"foo-inl.h\"                    // for FooFn\n"
            "#include \"foo.h\"                        // for FOO\n"
            "\n"
            "baz.cc should remove these lines:\n"
            "\n"
            "The full include-list for baz.cc:\n"
            "#include \"baz.h\"\n"
            "#include \"baz-inl.h\"\n"
            "#include <ctype.h>                      // for isascii\n"
            "#include <stdio.h>                      // for printf\n"
            "#include <sstring>                      // for sstring\n"
            "#include <string>                       // for string::iterator\n"
            "#include \"bar-inl.h\"                    // for BarFn\n"
            "#include \"bar.h\"                        // for BAR\n"
            "#include \"foo-inl.h\"                    // for FooFn\n"
            "#include \"foo.h\"                        // for FOO\n"
            "---\n",
            internal::PrintableDiffs("baz.cc", associated_includes, lines));
}

TEST(PrintableDiffsTest, ShowLineNumbersForDeletedIncludesEvenWithUses) {
  const set<string> empty;
  vector<OneIncludeOrForwardDeclareLine> lines;
  OneIncludeOrForwardDeclareLine line("\"foo.h\"", 1);
  line.set_present();   // *not* desired
  line.AddSymbolUse("Foo (ptr only)");
  lines.push_back(line);
  EXPECT_EQ("\n"
            "baz.cc should add these lines:\n"
            "\n"
            "baz.cc should remove these lines:\n"
            "- #include \"foo.h\"  // lines 1-1\n"
            "\n"
            "The full include-list for baz.cc:\n"
            "---\n",
            internal::PrintableDiffs("baz.cc", empty, lines));
}

#if 0

TEST_F(IwyuFileInfo_PrintableIncludeInformation, PrintsForwardDeclares) {
  IwyuFileInfo f("baz.cc");
  f.AddDesiredIncludeNeededBySymbol("\"foo.h\"", "FOO");
  f.AddDesiredIncludeNeededBySymbol("\"bar.h\"", "BAR");
  f.AddDesiredIncludeNeededBySymbol("\"baz.h\"", "BAZ");
  f.AddDesiredIncludeNeededBySymbol("<string>", "string::iterator");
  f.AddDesiredIncludeNeededBySymbol("<stdio.h>", "printf");
  FakeNamedDecl record1("struct", "Foo");
  f.AddDesiredForwardDeclare("\"record1.h\"", &record1);
  f.NormalizeDesiredSetsForCc(set<string>());
  EXPECT_EQ("\n"
            "baz.cc should add these lines:\n"
            "#include \"baz.h\"\n"
            "#include <stdio.h>                      // for printf\n"
            "#include <string>                       // for string::iterator\n"
            "#include \"bar.h\"                        // for BAR\n"
            "#include \"foo.h\"                        // for FOO\n"
            "struct Foo;\n"
            "\n"
            "baz.cc should remove these lines:\n"
            "\n"
            "The full include-list for baz.cc:\n"
            "#include \"baz.h\"\n"
            "#include <stdio.h>                      // for printf\n"
            "#include <string>                       // for string::iterator\n"
            "#include \"bar.h\"                        // for BAR\n"
            "#include \"foo.h\"                        // for FOO\n"
            "struct Foo;\n"
            "---\n",
            f.PrintableIncludeInformation());
}

using internal::PrintableIncludeOrForwardDeclareLine;

// This test fixture automatically saves/restores the verbose level
// between tests to prevent leaking side effects.
class PrintableIncludeOrForwardDeclareLineTest : public VerboseTest { };

TEST_F(PrintableIncludeOrForwardDeclareLineTest,
       ShowsReasonForNonMainCUHeaders) {
  // The reason should be printed even at a low verbose level.
  SetVerboseLevel(1);
  const char* symbols[] = { "Foo" };
  EXPECT_EQ("#include \"foo.h\"                        "
            "// for Foo\n",  // single symbol
            PrintableIncludeOrForwardDeclareLine(
                "bar.cc", MakeIncludeLine("\"foo.h\"", symbols)));
  const char* symbols2[] = { "Foo", "Bar" };
  EXPECT_EQ("#include \"foo.h\"                        "
            "// for Foo, Bar\n",  // multiple symbols
            PrintableIncludeOrForwardDeclareLine(
                "x.h", MakeIncludeLine("\"foo.h\"", symbols2)));
}

TEST_F(PrintableIncludeOrForwardDeclareLineTest, HidesReasonForMainCUHeaders) {
  // The reason should be hidden even at a high verbose level.
  SetVerboseLevel(5);
  const char* symbols[] = { "Foo" };
  EXPECT_EQ("#include \"foo.h\"\n",
            PrintableIncludeOrForwardDeclareLine(
                "foo.cc", MakeIncludeLine("\"foo.h\"", symbols)));
  EXPECT_EQ("#include \"foo.h\"\n",
            PrintableIncludeOrForwardDeclareLine(
                "foo_test.cc", MakeIncludeLine("\"foo.h\"", symbols)));
  EXPECT_EQ("#include \"foo-inl.h\"\n",
            PrintableIncludeOrForwardDeclareLine(
                "foo.cc", MakeIncludeLine("\"foo-inl.h\"", symbols)));
}

TEST_F(PrintableIncludeOrForwardDeclareLineTest,
       TruncatesLongSymbolListAtLowVerbosityLevels) {
  SetVerboseLevel(2);
  const char* symbols[] = { "Foo", "Bar", "SomeA", "SomeB", "SomeC", "BigX",
                            "BigY", "BigZ", "What", "Which", "Where" };
  EXPECT_EQ("#include <foo.h>         "
            "               // for Foo, Bar, SomeA, SomeB, etc\n",
            PrintableIncludeOrForwardDeclareLine(
                "bar.cc", MakeIncludeLine("<foo.h>", symbols)));

  EXPECT_EQ(
     "#include <some/really/really/really/really/long/path/abc.h>  "
     "// for Foo, etc\n",
     PrintableIncludeOrForwardDeclareLine(
         "a.cc",
         MakeIncludeLine("<some/really/really/really/really/long/path/abc.h>",
                         symbols)));

 EXPECT_EQ(
     // This line is exactly 80-character long, including the \n.
     "#include <some/really/really/really/really/long/path/abcdef.h>  "
     "// for Foo, etc\n",
     PrintableIncludeOrForwardDeclareLine(
         "foo.cc",
         MakeIncludeLine(
             "<some/really/really/really/really/long/path/abcdef.h>",
             symbols)));

 EXPECT_EQ(
     // There's no space on this line for " // for Foo, etc".  We
     // should cut off the comment entirely instead of printing
     // "  // for etc" or "  // for ".
     "#include <some/really/really/really/really/long/path/abcdefg.h>\n",
     PrintableIncludeOrForwardDeclareLine(
         "foo.cc",
         MakeIncludeLine(
             "<some/really/really/really/really/long/path/abcdefg.h>",
             symbols)));
}

TEST_F(PrintableIncludeOrForwardDeclareLineTest,
       PreservesLongSymbolListAtHighVerbosityLevels) {
  SetVerboseLevel(3);
  const char* symbols[] = { "Foo", "Bar", "SomeA", "SomeB", "SomeC", "BigX",
                            "BigY", "BigZ", "What", "Which", "Where" };
  EXPECT_EQ("#include <foo.h>                        "
            "// for Foo, Bar, SomeA, SomeB, SomeC, BigX, BigY, BigZ, What, "
            "Which, Where\n",
            PrintableIncludeOrForwardDeclareLine(
                "bar.cc", MakeIncludeLine("<foo.h>", symbols)));
}

TEST_F(PrintableIncludeOrForwardDeclareLineTest,
       OmitsReasonForSuperLongPathsAtLowVerbosityLevels) {
  SetVerboseLevel(2);
  const char* symbols[] = { "Foo" };
  EXPECT_EQ("#include \"some/really/really/really/really/really/deeply/buried/"
            "package/foo.h\"\n",
            PrintableIncludeOrForwardDeclareLine(
                "bar.cc",
                MakeIncludeLine("\"some/really/really/really/really/really/"
                                "deeply/buried/package/foo.h\"", symbols)));
}

TEST_F(PrintableIncludeOrForwardDeclareLineTest,
       PreservesReasonForSuperLongPathsAtHighVerbosityLevels) {
  SetVerboseLevel(3);
  const char* symbols[] = { "Foo" };
  EXPECT_EQ("#include \"some/really/really/really/really/really/deeply/buried/"
            "package/foo.h\"  // for Foo\n",
            PrintableIncludeOrForwardDeclareLine(
                "bar.cc",
                MakeIncludeLine("\"some/really/really/really/really/really/"
                                "deeply/buried/package/foo.h\"", symbols)));
}

TEST_F(PrintableIncludeOrForwardDeclareLineTest, PadsSpacesForShortPaths) {
  const char* symbols[] = { "Foo" };
  EXPECT_EQ("#include \"foo.h\"                        // for Foo\n",
            PrintableIncludeOrForwardDeclareLine(
                "bar.cc", MakeIncludeLine("\"foo.h\"", symbols)));
  EXPECT_EQ("#include \"foo/bar/b.h\"                  // for Foo\n",
            PrintableIncludeOrForwardDeclareLine(
                "bar.cc", MakeIncludeLine("\"foo/bar/b.h\"", symbols)));
}

TEST_F(PrintableIncludeOrForwardDeclareLineTest, DoesNotPadSpacesForLongPaths) {
  const char* symbols[] = { "Foo" };
  EXPECT_EQ("#include \"foo/bar/bluh.h\"               // for Foo\n",
            PrintableIncludeOrForwardDeclareLine(
                "bar.cc", MakeIncludeLine("\"foo/bar/bluh.h\"", symbols)));
  EXPECT_EQ("#include \"foo/bar/bluh-bluh.h\"          // for Foo\n",
            PrintableIncludeOrForwardDeclareLine(
                "bar.cc", MakeIncludeLine("\"foo/bar/bluh-bluh.h\"", symbols)));
}

TEST_F(PrintableIncludeOrForwardDeclareLineTest,
       PrintsLineNumbersWhenThereIsNoUsedSymbol) {
  IncludeOrForwardDeclareLineData data;
  data.line = "#include <foo.h>";
  data.line_num_range = make_pair(12, 12);
  // data.used_symbols is intentionally left empty.
  // We don't really care about .is_present and .is_desired.
  EXPECT_EQ("#include <foo.h>  // lines 12-12\n",
            PrintableIncludeOrForwardDeclareLine("bar.cc", data));
}

TEST_F(PrintableIncludeOrForwardDeclareLineTest,
       PrintsForwardDeclaresProperlyWithLineNumbers) {
  IncludeOrForwardDeclareLineData data;
  data.line = "class Foo;";
  data.line_num_range = make_pair(7, 8);
  EXPECT_EQ("class Foo;  // lines 7-8\n",
            PrintableIncludeOrForwardDeclareLine("bar.cc", data));
}

TEST_F(PrintableIncludeOrForwardDeclareLineTest,
       PrintsForwardDeclaresProperlyWithNoLineNumbers) {
  IncludeOrForwardDeclareLineData data;
  data.line = "class Foo;";
  data.line_num_range = make_pair(-1, -1);
  EXPECT_EQ("class Foo;\n",
            PrintableIncludeOrForwardDeclareLine("bar.cc", data));
}

#endif

}  // unnamed namespace
}  // namespace include_what_you_use
