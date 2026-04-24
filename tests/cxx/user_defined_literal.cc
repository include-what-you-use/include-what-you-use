//===--- user_defined_literal.cc - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

#include "user_defined_literal-direct.h"

class IndirectClass;
IndirectClass operator""_ic(unsigned long long);

// IWYU: operator""_x(unsigned long long) is...*user_defined_literal-indirect.h
constexpr unsigned long long kLiteralValue = 1_x;

void Fn() {
  // IWYU: IndirectClass is...*user_defined_literal-indirect.h
  (void)1_ic;
}

int main() { return kLiteralValue; }

/**** IWYU_SUMMARY

tests/cxx/user_defined_literal.cc should add these lines:
#include "tests/cxx/user_defined_literal-indirect.h"

tests/cxx/user_defined_literal.cc should remove these lines:
- #include "user_defined_literal-direct.h"  // lines XX-XX
- class IndirectClass;  // lines XX-XX

The full include-list for tests/cxx/user_defined_literal.cc:
#include "tests/cxx/user_defined_literal-indirect.h"  // for IndirectClass, operator""_x

***** IWYU_SUMMARY */
