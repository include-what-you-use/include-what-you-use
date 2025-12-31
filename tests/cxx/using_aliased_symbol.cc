//===--- using_aliased_symbol.cc - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests that when we include a file that exports a symbol via a using decl
// that we are required to include both the file with the original symbol and
// the file with the using decl.

#include "tests/cxx/direct.h"
#include "tests/cxx/using_aliased_symbol-alias.h"

void use_symbol() {
  // IWYU: ns::symbol() is...*using_aliased_symbol-declare.h
  ns2::symbol();
  // Use of non-providing typedef requires full underlying type info to be
  // provided by the user. Hence, full IndirectClass info is required here.
  // IWYU: ns::Typedef is...*using_aliased_symbol-declare.h
  // IWYU: IndirectClass is...*indirect.h
  ns2::Typedef a;
  // Typedef use in a fwd-decl context doesn't require underlying type info.
  // IWYU: ns::Typedef is...*using_aliased_symbol-declare.h
  ns2::Typedef* pa;
}

/**** IWYU_SUMMARY


tests/cxx/using_aliased_symbol.cc should add these lines:
#include "tests/cxx/indirect.h"
#include "tests/cxx/using_aliased_symbol-declare.h"

tests/cxx/using_aliased_symbol.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/using_aliased_symbol.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass
#include "tests/cxx/using_aliased_symbol-alias.h"  // for Typedef, symbol
#include "tests/cxx/using_aliased_symbol-declare.h"  // for Typedef, symbol

***** IWYU_SUMMARY */
