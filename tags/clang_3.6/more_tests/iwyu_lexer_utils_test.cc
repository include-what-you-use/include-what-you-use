//===--- iwyu_lexer_utils_test.cc - test iwyu_lexer_utils.{cc,h} ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


// Tests for the iwyu_lexer_utils module. In addition, provides sample
// code for using Clang's Lexer.

#include "iwyu_lexer_utils.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "base/logging.h"
#include "iwyu_globals.h"
#include "testing/base/public/gunit.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/Token.h"

using clang::LangOptions;
using clang::Lexer;
using clang::SourceLocation;
using clang::SourceRange;
using clang::Token;

namespace iwyu = include_what_you_use;
using iwyu::CharacterDataGetterInterface;
using iwyu::FindArgumentsToDefined;

namespace {

// Hack. Work around the fact that only SourceManagers can create
// non-trivial SourceLocations.
SourceLocation CreateSourceLocationFromOffset(SourceLocation begin_loc,
                                              unsigned offset) {
  return SourceLocation::getFromRawEncoding(begin_loc.getRawEncoding()
                                            + offset);
}

class StringCharacterDataGetter : public CharacterDataGetterInterface {
 public:
  StringCharacterDataGetter(const string& str)
      : str_("unused" + str) {
  }

  virtual const char* GetCharacterData(SourceLocation loc) const {
    unsigned offset = loc.getRawEncoding();
    CHECK_LE(offset, str_.size());
    return str_.c_str() + offset;
  }

  SourceLocation BeginningOfString() {
    // Returns an index into str that skips over the "unused" set in the ctor.
    return CreateSourceLocationFromOffset(SourceLocation(), strlen("unused"));
  }

 private:
  string str_;
};

TEST(LexerTest, ClangLexer) {
  // Not so much a test as an example of how to use the Lexer.
  const char text[] = "#if defined(FOO)";
  Lexer lexer(SourceLocation(), LangOptions(), text, text, text + strlen(text));

  Token token;
  while (!lexer.LexFromRawLexer(token)) {
    printf("Token: %s at %u length %u: \"%s\"\n",
           token.getName(), token.getLocation().getRawEncoding(),
           token.getLength(),
           string(text + token.getLocation().getRawEncoding(),
                  token.getLength()).c_str());
  }
}

// Common test code for testing FindArgumentsToDefined. The symbols
// should be the arguments to defined() in order.
void TestFindArgumentsToDefinedWithText(const string& text,
                                        const vector<string>& symbols) {
  StringCharacterDataGetter data_getter(text);
  SourceLocation begin_loc = data_getter.BeginningOfString();
  SourceLocation end_loc =
      CreateSourceLocationFromOffset(begin_loc, text.size());

  vector<Token> defined_tokens = FindArgumentsToDefined(
      SourceRange(begin_loc, end_loc), data_getter);
  EXPECT_EQ(symbols.size(), defined_tokens.size());
  for (int i = 0; i < symbols.size(); ++i) {
    const string& symbol = symbols[i];
    const Token& token = defined_tokens[i];
    EXPECT_EQ(clang::tok::raw_identifier, token.getKind());
    SourceLocation expected_loc =
        CreateSourceLocationFromOffset(begin_loc, text.find(symbol));
    EXPECT_EQ(expected_loc, token.getLocation());
    EXPECT_EQ(symbol.size(), token.getLength());
    EXPECT_EQ(symbol, GetTokenText(token, data_getter));
  }
}

TEST(FindArgumentsToDefined, InParentheses) {
  vector<string> symbols;
  symbols.push_back("FOO");
  TestFindArgumentsToDefinedWithText("#if defined(FOO)\n", symbols);
}

TEST(FindArgumentsToDefined, NoParentheses) {
  vector<string> symbols;
  symbols.push_back("FOO");
  TestFindArgumentsToDefinedWithText("#if defined FOO\n", symbols);
}

TEST(FindArgumentsToDefined, MultipleArgs) {
  vector<string> symbols;
  symbols.push_back("FOO");
  symbols.push_back("BAR");
  symbols.push_back("BAZ");
  TestFindArgumentsToDefinedWithText(
      "#if defined FOO || defined(BAR) || !defined(BAZ)\n", symbols);
}

TEST(GetSourceTextUntilEndOfLine, FullLine) {
  const char text[] = "This is the full line.\n";
  StringCharacterDataGetter data_getter(text);
  SourceLocation begin_loc = data_getter.BeginningOfString();
  EXPECT_EQ("This is the full line.",
            GetSourceTextUntilEndOfLine(begin_loc, data_getter));
}

TEST(GetSourceTextUntilEndOfLine, MultipleLines) {
  const char text[] = "This is the full line.\nThis line should be ignored.\n";
  StringCharacterDataGetter data_getter(text);
  SourceLocation begin_loc = data_getter.BeginningOfString();
  EXPECT_EQ("This is the full line.",
            GetSourceTextUntilEndOfLine(begin_loc, data_getter));
}

TEST(GetSourceTextUntilEndOfLine, PartialLine) {
  const char text[] = "This is the full line.\n";
  StringCharacterDataGetter data_getter(text);
  SourceLocation begin_loc = data_getter.BeginningOfString();
  SourceLocation middle_loc = CreateSourceLocationFromOffset(begin_loc, 5);
  EXPECT_EQ("is the full line.",
            GetSourceTextUntilEndOfLine(middle_loc, data_getter));
}

TEST(GetSourceTextUntilEndOfLine, MiddleLine) {
  const char text[] = "This is the full line.\nThis is the winning line.\n";
  StringCharacterDataGetter data_getter(text);
  SourceLocation begin_loc = data_getter.BeginningOfString();
  SourceLocation middle_loc = CreateSourceLocationFromOffset(begin_loc, 35);
  EXPECT_EQ("winning line.",
            GetSourceTextUntilEndOfLine(middle_loc, data_getter));
}

TEST(GetSourceTextUntilEndOfLine, NoNewline) {
  const char text[] = "This is the full line.";
  StringCharacterDataGetter data_getter(text);
  SourceLocation begin_loc = data_getter.BeginningOfString();
  EXPECT_EQ("This is the full line.",
            GetSourceTextUntilEndOfLine(begin_loc, data_getter));
}

TEST(GetLocationAfter, FullLine) {
  const char text[] = "This is the full line.\n";
  StringCharacterDataGetter data_getter(text);
  SourceLocation begin_loc = data_getter.BeginningOfString();
  SourceLocation after_loc = GetLocationAfter(begin_loc, "is the", data_getter);
  EXPECT_TRUE(after_loc.isValid());
  // We can't explore after_loc directly (it's opaque), so we use
  // GetSourceTextUntilEndOfLine as a proxy.
  EXPECT_EQ(" full line.", GetSourceTextUntilEndOfLine(after_loc, data_getter));
}

TEST(GetLocationAfter, FirstOfManyOccurrences) {
  const char text[] = "This is the full line.\nThis is the full line too.\n";
  StringCharacterDataGetter data_getter(text);
  SourceLocation begin_loc = data_getter.BeginningOfString();
  SourceLocation after_loc = GetLocationAfter(begin_loc, "is the", data_getter);
  EXPECT_TRUE(after_loc.isValid());
  EXPECT_EQ(" full line.", GetSourceTextUntilEndOfLine(after_loc, data_getter));
}

TEST(GetLocationAfter, SecondOfManyOccurrences) {
  const char text[] = "This is the full line.\nThis is the full line too.\n";
  StringCharacterDataGetter data_getter(text);
  SourceLocation begin_loc = data_getter.BeginningOfString();
  SourceLocation after_loc = GetLocationAfter(begin_loc, "is the", data_getter);
  EXPECT_TRUE(after_loc.isValid());
  after_loc = GetLocationAfter(after_loc, "is the", data_getter);
  EXPECT_TRUE(after_loc.isValid());
  EXPECT_EQ(" full line too.",
            GetSourceTextUntilEndOfLine(after_loc, data_getter));
}

TEST(GetLocationAfter, NeedleNotFound) {
  const char text[] = "This is the full line.\nThis is the full line too.\n";
  StringCharacterDataGetter data_getter(text);
  SourceLocation begin_loc = data_getter.BeginningOfString();
  SourceLocation after_loc = GetLocationAfter(begin_loc, "isthe", data_getter);
  EXPECT_FALSE(after_loc.isValid());
}

TEST(GetLocationAfter, NeedleNotFoundTwice) {
  const char text[] = "This is the full line.\nThis is the full line too.\n";
  StringCharacterDataGetter data_getter(text);
  SourceLocation begin_loc = data_getter.BeginningOfString();
  SourceLocation after_loc = GetLocationAfter(begin_loc, "line.", data_getter);
  EXPECT_TRUE(after_loc.isValid());
  after_loc = GetLocationAfter(after_loc, "line.", data_getter);
  EXPECT_FALSE(after_loc.isValid());
}

TEST(GetLocationAfter, EmptyNeedle) {
  const char text[] = "This is the full line.";
  StringCharacterDataGetter data_getter(text);
  SourceLocation begin_loc = data_getter.BeginningOfString();
  SourceLocation after_loc = GetLocationAfter(begin_loc, "", data_getter);
  EXPECT_TRUE(after_loc.isValid());
  EXPECT_EQ("This is the full line.",
            GetSourceTextUntilEndOfLine(after_loc, data_getter));
}

TEST(GetLocationAfter, BeginAfterStartOfText) {
  const char text[] = "This is the full line.  This is the second 'this'.";
  StringCharacterDataGetter data_getter(text);
  SourceLocation begin_loc = data_getter.BeginningOfString();
  begin_loc = CreateSourceLocationFromOffset(begin_loc, 1);
  SourceLocation after_loc = GetLocationAfter(begin_loc, "This", data_getter);
  EXPECT_TRUE(after_loc.isValid());
  EXPECT_EQ(" is the second 'this'.",
            GetSourceTextUntilEndOfLine(after_loc, data_getter));
}

TEST(GetIncludeNameAsTyped, SystemInclude) {
  const char text[] = "#include <stdio.h>\n";
  StringCharacterDataGetter data_getter(text);
  SourceLocation begin_loc = data_getter.BeginningOfString();
  SourceLocation inc_loc = CreateSourceLocationFromOffset(begin_loc, 9);
  EXPECT_EQ("<stdio.h>",
            GetIncludeNameAsTyped(inc_loc, data_getter));
}

TEST(GetIncludeNameAsTyped, NonsysytemInclude) {
  const char text[] = "#include \"ads/util.h\"\n";
  StringCharacterDataGetter data_getter(text);
  SourceLocation begin_loc = data_getter.BeginningOfString();
  SourceLocation inc_loc = CreateSourceLocationFromOffset(begin_loc, 9);
  EXPECT_EQ("\"ads/util.h\"",
            GetIncludeNameAsTyped(inc_loc, data_getter));
}

TEST(GetIncludeNameAsTyped, WithComments) {
  const char text[] = "#include <stdio.h>  // for printf\n";
  StringCharacterDataGetter data_getter(text);
  SourceLocation begin_loc = data_getter.BeginningOfString();
  SourceLocation inc_loc = CreateSourceLocationFromOffset(begin_loc, 9);
  EXPECT_EQ("<stdio.h>",
            GetIncludeNameAsTyped(inc_loc, data_getter));
}

}  // namespace


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  include_what_you_use::InitGlobalsAndFlagsForTesting();
  return RUN_ALL_TESTS();
}
