//===--- depopulated_h_file.cc - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// The directly-included file contains only an inclusion of the file
// defining Foo. IWYU should recommend that inclusion be moved to this file.
#include "tests/depopulated_h_file.h"

void foo() {
  Foo::Bar();
}

/**** IWYU_SUMMARY

tests/depopulated_h_file.cc should add these lines:
#include "tests/depopulated_h_file-i1.h"

tests/depopulated_h_file.cc should remove these lines:

The full include-list for tests/depopulated_h_file.cc:
#include "tests/depopulated_h_file.h"
#include "tests/depopulated_h_file-i1.h"  // for Foo

***** IWYU_SUMMARY */
