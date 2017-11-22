//===--- no_fwd_decls.cc - test input file for iwyu -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Test that passing the --no_fwd_decls switch to IWYU suggests including the
// corresponding header file even when the use is not a full use.

#include "tests/cxx/direct.h"

// IWYU: IndirectClass is...*indirect.h
IndirectClass* global;

/**** IWYU_SUMMARY

tests/cxx/no_fwd_decls.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/no_fwd_decls.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/no_fwd_decls.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
