//===--- provided_functions-direct.h - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/provided_functions-i1.h"

// IWYU: Indirect is...*-i2.h
Indirect GetProvided();

// IWYU: Indirect is...*-i2.h
using ProvidingAlias = Indirect;
// IWYU: Unrelated is...*-i2.h
using ProvidingUnrelated = Unrelated;

// IWYU: Indirect is...*-i2.h
struct IndirectDerived : Indirect {};

/**** IWYU_SUMMARY

tests/cxx/provided_functions-direct.h should add these lines:
#include "tests/cxx/provided_functions-i2.h"

tests/cxx/provided_functions-direct.h should remove these lines:
- #include "tests/cxx/provided_functions-i1.h"  // lines XX-XX

The full include-list for tests/cxx/provided_functions-direct.h:
#include "tests/cxx/provided_functions-i2.h"  // for Indirect, Unrelated

***** IWYU_SUMMARY */
