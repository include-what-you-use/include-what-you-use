//===--- pragma_always_keep.cc - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This test demonstrates that the "always_keep" pragma forces an includer to
// use a nominally unused file.

// IWYU_ARGS: -I .

#include "tests/cxx/pragma_always_keep-d1.h"

// direct.h does not have an "always_keep" pragma, and should be removed.
#include "tests/cxx/direct.h"

/**** IWYU_SUMMARY
tests/cxx/pragma_always_keep.cc should add these lines:

tests/cxx/pragma_always_keep.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/pragma_always_keep.cc:
#include "tests/cxx/pragma_always_keep-d1.h"

***** IWYU_SUMMARY */
