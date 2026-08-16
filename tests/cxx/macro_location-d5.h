//===--- macro_location-d5.h - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This tests that IWYU doesn't require keeping "-d7.h" header here just because
// an AST node corresponding to '1' is macro-expanded there, whereas its parent
// BinaryOperator AST node is located in this file. However, this is doubtful:
// it could make sense to keep "-d7.h" here because it (indirectly) uses
// the macro TWO defined by includer (i.e. this file).

#define TWO 1 + 1

#include "tests/cxx/macro_location-d7.h"

/**** IWYU_SUMMARY

tests/cxx/macro_location-d5.h should add these lines:

tests/cxx/macro_location-d5.h should remove these lines:
- #include "tests/cxx/macro_location-d7.h"  // lines XX-XX

The full include-list for tests/cxx/macro_location-d5.h:

***** IWYU_SUMMARY */
