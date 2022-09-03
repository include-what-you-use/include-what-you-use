//===--- iwyu_regex.cc - iwyu regex implementation ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_regex.h"

#include <regex>
#include "llvm/Support/Regex.h"

#include "iwyu_port.h"

namespace include_what_you_use {

bool ParseRegexDialect(const char* str, RegexDialect* dialect) {
  if (strcmp(str, "llvm") == 0) {
    *dialect = RegexDialect::LLVM;
    return true;
  } else if (strcmp(str, "ecmascript") == 0) {
    *dialect = RegexDialect::ECMAScript;
    return true;
  }
  return false;
}

// Returns true if str matches regular expression pattern for the given dialect.
bool RegexMatch(RegexDialect dialect, const std::string& str,
                const std::string& pattern) {
  switch (dialect) {
    case RegexDialect::LLVM: {
      // llvm::Regex::match has search semantics. Enclose the pattern in ^...$
      // for start/end anchoring.
      llvm::Regex r("^" + pattern + "$");
      return r.match(str);
    }

    case RegexDialect::ECMAScript: {
      std::regex r(pattern, std::regex_constants::ECMAScript);
      return std::regex_match(str, r);
    }
  }
  CHECK_UNREACHABLE_("Unexpected regex dialect");
}

}  // namespace include_what_you_use
