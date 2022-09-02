//===--- iwyu_regex.h - iwyu regex implementation -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_IWYU_REGEX_H_
#define INCLUDE_WHAT_YOU_USE_IWYU_REGEX_H_

#include <string>

namespace include_what_you_use {

enum class RegexDialect { LLVM = 0, ECMAScript = 1 };

// Parse dialect string to enum.
bool ParseRegexDialect(const char* str, RegexDialect* dialect);

// Returns true if str matches regular expression pattern for the given dialect.
bool RegexMatch(RegexDialect dialect, const std::string& str,
                const std::string& pattern);

}  // namespace include_what_you_use

#endif  // INCLUDE_WHAT_YOU_USE_IWYU_REGEX_H_
