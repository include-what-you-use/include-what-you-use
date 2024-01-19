//===--- extern_variable.cc - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -Wno-extern-initializer

// Tests handling variables marked as 'extern'.

#include "tests/cxx/direct.h"

// IWYU: IndirectClass needs a declaration
extern IndirectClass forward_declarable;

// IWYU: IndirectClass is...*indirect.h
extern IndirectClass initialized = {1};

/**** IWYU_SUMMARY

tests/cxx/extern_variable.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/extern_variable.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/extern_variable.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
