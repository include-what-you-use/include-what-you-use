//===--- using_import_unused.cc - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "using_import-import.h"

void use_symbol() {  

}

/**** IWYU_SUMMARY

tests/cxx/using_import_unused.cc should add these lines:

tests/cxx/using_import_unused.cc should remove these lines:
- #include "using_import-import.h"  // lines XX-XX

The full include-list for tests/cxx/using_import_unused.cc:

***** IWYU_SUMMARY */
