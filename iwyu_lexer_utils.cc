//===--- iwyu_lexer_utils.cc - clang-lexer utilities for iwyu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_lexer_utils.h"
#include "iwyu_globals.h"
#include "iwyu_port.h"  // for CHECK_

#include <string>

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Token.h"

using clang::SourceLocation;
using clang::SourceManager;
using clang::SourceRange;
using clang::Token;
using llvm::StringRef;
using std::string;

namespace include_what_you_use {

bool LineHasText(SourceLocation source_location, StringRef text) {
  const StringRef data =
      GetSourceTextUntilEndOfLine(source_location, DefaultDataGetter());
  return data.find(text) != StringRef::npos;
}

// SourceManagerCharacterDataGetter method implementations.
SourceManagerCharacterDataGetter::SourceManagerCharacterDataGetter(
    const SourceManager& source_manager)
    : source_manager_(source_manager) {
}

const char* SourceManagerCharacterDataGetter::GetCharacterData(
    SourceLocation loc) const {
  bool invalid;
  const char* data = source_manager_.getCharacterData(loc, &invalid);
  CHECK_(!invalid);
  CHECK_(data);
  return data;
}

StringRef GetSourceTextUntilEndOfLine(
    SourceLocation start_loc, const CharacterDataGetterInterface& data_getter) {
  const char* data = data_getter.GetCharacterData(start_loc);
  const char* line_end = strchr(data, '\n');
  if (!line_end)
    return data;
  return StringRef(data, line_end - data);
}

SourceLocation GetLocationAfter(
    SourceLocation start_loc, const string& needle,
    const CharacterDataGetterInterface& data_getter) {
  CHECK_(start_loc.isValid() && "GetLocationAfter takes only valid locations");
  const char* data = data_getter.GetCharacterData(start_loc);
  const char* needle_loc = strstr(data, needle.c_str());
  if (!needle_loc)
    return SourceLocation();   // invalid source location
  return start_loc.getLocWithOffset(needle_loc - data + needle.length());
}

string GetIncludeNameAsWritten(
    SourceLocation include_loc,
    const CharacterDataGetterInterface& data_getter) {
  const string data = GetSourceTextUntilEndOfLine(include_loc, data_getter).str();
  if (data.empty())
    return data;
  string::size_type endpos = string::npos;
  if (data[0] == '<') {
    endpos = data.find('>', 1);
  } else if (data[0] == '"') {
    endpos = data.find('"', 1);
  } else {
    CHECK_UNREACHABLE_("Unexpected token being #included");
  }
  CHECK_(endpos != string::npos && "No end-character found for #include");
  return data.substr(0, endpos+1);
}

// Get the text of a given token.
string GetTokenText(const Token& token,
                    const CharacterDataGetterInterface& data_getter) {
  const char* text = data_getter.GetCharacterData(token.getLocation());
  return string(text, token.getLength());
}

}  // namespace include_what_you_use
