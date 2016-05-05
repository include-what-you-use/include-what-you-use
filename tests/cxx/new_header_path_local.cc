//===--- new_header_path_local.cc - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that if include search path is not provided, new includes are added
// without path (just file name). Compare with new_header_path_provided.cc.

#include "direct_near.h"

void foo() {
  // IWYU: IndirectClass is...*indirect.h
  IndirectClass ic;
}

/**** IWYU_SUMMARY

tests/cxx/new_header_path_local.cc should add these lines:
#include "indirect.h"

tests/cxx/new_header_path_local.cc should remove these lines:
- #include "direct_near.h"  // lines XX-XX

The full include-list for tests/cxx/new_header_path_local.cc:
#include "indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
