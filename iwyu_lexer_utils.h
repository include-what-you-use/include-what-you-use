//===--- iwyu_lexer_utils.h - clang-lexer utilities for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_IWYU_LEXER_UTILS_H_
#define INCLUDE_WHAT_YOU_USE_IWYU_LEXER_UTILS_H_

#include <string>                       // for string
#include <vector>                       // for vector

#include "clang/Basic/SourceLocation.h"

namespace clang {
class SourceManager;
class Token;
}  // namespace clang

namespace include_what_you_use {

using std::string;
using std::vector;

// Interface to get character data from a SourceLocation. This allows
// tests to avoid constructing a SourceManager yet still allow iwyu to
// get the character data from SourceLocations.
class CharacterDataGetterInterface {
 public:
  virtual ~CharacterDataGetterInterface() {}
  virtual const char* GetCharacterData(clang::SourceLocation loc) const = 0;
};

// Implementation of CharacterDataGetterInterface that uses a SourceManager.
class SourceManagerCharacterDataGetter : public CharacterDataGetterInterface {
 public:
  explicit SourceManagerCharacterDataGetter(
      const clang::SourceManager& source_manager);
  const char* GetCharacterData(clang::SourceLocation loc) const override;

 private:
  const clang::SourceManager& source_manager_;
};

// Returns the source-code line from the current location until \n.
string GetSourceTextUntilEndOfLine(
    clang::SourceLocation start_loc,
    const CharacterDataGetterInterface& data_getter);

// Returns the location right *after* the first occurrence of needle
// after start_loc, if any.  (If none, returns an invalid source-loc.)
// start_loc must be a valid source location.
clang::SourceLocation GetLocationAfter(
    clang::SourceLocation start_loc, const string& needle,
    const CharacterDataGetterInterface& data_getter);

// Returns the include-name as written, including <>'s and ""'s.
// Resolved computed includes first, so given
//    #define INC  <stdio.h>
//    #include INC
// If include_loc points to the second INC, we'll return '<stdio.h>'.
string GetIncludeNameAsWritten(
    clang::SourceLocation include_loc,
    const CharacterDataGetterInterface& data_getter);

// Given the range of an #if or #elif statement, determine the
// symbols which are arguments to "defined". This allows iwyu to
// treat these symbols as if #ifdef was used instead.
vector<clang::Token> FindArgumentsToDefined(
    clang::SourceRange range,
    const CharacterDataGetterInterface& data_getter);

// Get the text of a given token.
string GetTokenText(const clang::Token& token,
                    const CharacterDataGetterInterface& data_getter);

}  // namespace include_what_you_use

#endif  // INCLUDE_WHAT_YOU_USE_IWYU_LEXER_UTILS_H_
