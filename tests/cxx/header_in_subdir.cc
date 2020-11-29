//===--- header_in_subdir.cc - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Test that subdirs are correctly recognized

#include "subdir/direct_subdir.h"

void foo() {
  // IWYU: IndirectSubDirClass is...*indirect_subdir.h
  IndirectSubDirClass ic;
}

/**** IWYU_SUMMARY

tests/cxx/header_in_subdir.cc should add these lines:
#include "tests/cxx/subdir/indirect_subdir.h"

tests/cxx/header_in_subdir.cc should remove these lines:
- #include "subdir/direct_subdir.h"  // lines XX-XX

The full include-list for tests/cxx/header_in_subdir.cc:
#include "tests/cxx/subdir/indirect_subdir.h"  // for IndirectSubDirClass

***** IWYU_SUMMARY */
