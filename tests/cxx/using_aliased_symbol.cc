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

#include "using_aliased_symbol-alias.h"

void use_symbol() {  
  // IWYU: ns::symbol is defined in ...*using_aliased_symbol-declare.h", which isn't directly #included.
  ns2::symbol();
}

/**** IWYU_SUMMARY


tests/cxx/using_aliased_symbol.cc should add these lines:
#include "tests/cxx/using_aliased_symbol-declare.h"

tests/cxx/using_aliased_symbol.cc should remove these lines:

The full include-list for tests/cxx/using_aliased_symbol.cc:
#include "tests/cxx/using_aliased_symbol-declare.h"  // for symbol
#include "using_aliased_symbol-alias.h"  // for symbol

***** IWYU_SUMMARY */
