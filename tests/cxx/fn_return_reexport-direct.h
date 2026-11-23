//===--- fn_return_reexport-direct.h - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/direct.h"

// IWYU: IndirectClass is...*indirect.h
IndirectClass GetProvidedIndirectClass();

/**** IWYU_SUMMARY

tests/cxx/fn_return_reexport-direct.h should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/fn_return_reexport-direct.h should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/fn_return_reexport-direct.h:
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
