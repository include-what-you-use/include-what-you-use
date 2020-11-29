//===--- using_aliased_symbol_unused.cc - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests that when we include a file that contains a using decl, that we are not
// forced to include that file if the decl is not used.

#include "using_aliased_symbol-alias.h"

void not_use_symbol() {  

}

/**** IWYU_SUMMARY

tests/cxx/using_aliased_symbol_unused.cc should add these lines:

tests/cxx/using_aliased_symbol_unused.cc should remove these lines:
- #include "using_aliased_symbol-alias.h"  // lines XX-XX

The full include-list for tests/cxx/using_aliased_symbol_unused.cc:

***** IWYU_SUMMARY */
