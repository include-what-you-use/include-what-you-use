//===--- new_header_path_provided.cc - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests that if include search path is provided, new includes are added with
// corresponding relative path. Compare with new_header_path_local.cc.

#include "tests/cxx/direct.h"

void foo() {
  // IWYU: IndirectClass is...*indirect.h
  IndirectClass ic;
}

/**** IWYU_SUMMARY

tests/cxx/new_header_path_provided.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/new_header_path_provided.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/new_header_path_provided.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
