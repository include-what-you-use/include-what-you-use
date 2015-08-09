//===--- using_import.cc - test input file for iwyu -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "using_import-import.h"

void use_symbol() {  
  // IWYU: ns::symbol is defined in ...*using_import-declare.h", which isn't directly #included.
  symbol();
}

/**** IWYU_SUMMARY


tests/cxx/using_import.cc should add these lines:
#include "tests/cxx/using_import-declare.h"

tests/cxx/using_import.cc should remove these lines:

The full include-list for tests/cxx/using_import.cc:
#include "tests/cxx/using_import-declare.h"  // for symbol
#include "using_import-import.h"  // for symbol

***** IWYU_SUMMARY */
