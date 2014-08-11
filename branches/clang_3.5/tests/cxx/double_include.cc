//===--- double_include.cc - test input file for iwyu ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that when we include a file twice (that doesn't have a header
// guard), we don't get confused and ask it to delete a copy of all
// *its* includes, thinking it specifies each include twice.
//
// That is, we're only testing in passing that iwyu tells us to remove
// one of the double_include.h's below.  What we really care about is
// that iwyu doesn't think that double_include.h is #including
// indirect.h twice, just because we see that include path twice (due
// to the double-include here, and the fact double_include.h lacks a
// header-guard).

#include "tests/cxx/double_include.h"
#include "tests/cxx/double_include.h"

IndirectClass ic;

/**** IWYU_SUMMARY

tests/cxx/double_include.cc should add these lines:

tests/cxx/double_include.cc should remove these lines:
- #include "tests/cxx/double_include.h"  // lines XX-XX

The full include-list for tests/cxx/double_include.cc:
#include "tests/cxx/double_include.h"

***** IWYU_SUMMARY */
