//===--- using_import.cc - test input file for iwyu -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "import_symbol.h"

void use_symbol() {  
  symbol();
}

/**** IWYU_SUMMARY

tests/cxx/using_import.cc should add these lines:
#include "declare_symbol.h" // lines XX-XX

tests/cxx/using_import.cc should remove these lines:

The full include-list for tests/cxx/using_import.cc:
#include "import_symbol.h"  // for symbol
#include "tests/cxx/declare_symbol.h"  // for symbol

***** IWYU_SUMMARY */
