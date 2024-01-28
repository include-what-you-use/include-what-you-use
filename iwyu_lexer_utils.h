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

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/TokenKinds.h"
#include "clang/Lex/MacroInfo.h"
#include "llvm/ADT/StringRef.h"

namespace clang {
class SourceManager;
class Token;
}  // namespace clang

namespace include_what_you_use {

using std::string;

// For a particular source line that source_location points to,
// returns true if the given text occurs on the line.
// (Case sensitive.)
bool LineHasText(clang::SourceLocation source_location, llvm::StringRef text);

// Interface to get character data from a SourceLocation. This allows
// tests to avoid constructing a SourceManager yet still allow iwyu to
// get the character data from SourceLocations.
class CharacterDataGetterInterface {
 public:
  virtual ~CharacterDataGetterInterface() = default;
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
llvm::StringRef GetSourceTextUntilEndOfLine(
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

// Get the text of a given token.
string GetTokenText(const clang::Token& token,
                    const CharacterDataGetterInterface& data_getter);

// Given a token iterator inside a macro, returns the kind of the token after
// current, ignoring comment tokens. If there is no token after, returns
// clang::tok::unknown.
clang::tok::TokenKind GetNextMacroTokenKind(
    const clang::MacroInfo* macro,
    clang::MacroInfo::const_tokens_iterator current);

}  // namespace include_what_you_use

#endif  // INCLUDE_WHAT_YOU_USE_IWYU_LEXER_UTILS_H_
