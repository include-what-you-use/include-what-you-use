//===--- iwyu_regex.cc - iwyu regex implementation ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_regex.h"

#include <cstring>
#include <regex>

#include "iwyu_port.h"
#include "iwyu_string_util.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Regex.h"

namespace include_what_you_use {

namespace {

// Add ^...$ start/end anchoring if they don't already exist.
// This is useful to transform from search-inside-string semantics to match-
// whole-string semantics for regex implementations that don't support the
// latter.
std::string Anchored(const std::string& pattern) {
  const char* prefix = "";
  const char* suffix = "";
  if (!StartsWith(pattern, "^")) {
    prefix = "^";
  }
  if (!EndsWith(pattern, "$")) {
    suffix = "$";
  }

  return prefix + pattern + suffix;
}

}  // anonymous namespace

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

bool RegexMatch(RegexDialect dialect, const std::string& str,
                const std::string& pattern) {
  switch (dialect) {
    case RegexDialect::LLVM: {
      // llvm::Regex::match has search semantics; ensure anchored.
      llvm::Regex r(Anchored(pattern));
      return r.match(str);
    }

    case RegexDialect::ECMAScript: {
      std::regex r(pattern, std::regex_constants::ECMAScript);
      return std::regex_match(str, r);
    }
  }
  CHECK_UNREACHABLE_("Unexpected regex dialect");
}

std::string RegexReplace(RegexDialect dialect, const std::string& str,
                         const std::string& pattern,
                         const std::string& replacement) {
  switch (dialect) {
    case RegexDialect::LLVM: {
      // llvm::Regex::sub has search semantics; ensure anchored.
      llvm::Regex r(Anchored(pattern));
      return r.sub(replacement, str);
    }

    case RegexDialect::ECMAScript: {
      // std::regex_replace has search semantics; ensure anchored.
      std::regex r(Anchored(pattern), std::regex_constants::ECMAScript);
      return std::regex_replace(str, r, replacement,
                                std::regex_constants::format_first_only);
    }
  }
  CHECK_UNREACHABLE_("Unexpected regex dialect");
}

}  // namespace include_what_you_use
