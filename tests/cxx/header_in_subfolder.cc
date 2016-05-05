//===--- header_in_subfolder.cc - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Test that subfolders are correctly recognized

#include "subfolder/direct_subfolder.h"

void foo() {
  // IWYU: IndirectSubfolderClass is...*indirect_subfolder.h
  IndirectSubfolderClass ic;
}

/**** IWYU_SUMMARY

tests/cxx/header_in_subfolder.cc should add these lines:
#include "tests/cxx/subfolder/indirect_subfolder.h"

tests/cxx/header_in_subfolder.cc should remove these lines:
- #include "subfolder/direct_subfolder.h"  // lines XX-XX

The full include-list for tests/cxx/header_in_subfolder.cc:
#include "tests/cxx/subfolder/indirect_subfolder.h"  // for IndirectSubfolderClass

***** IWYU_SUMMARY */
