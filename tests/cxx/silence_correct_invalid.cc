//===--- silence_correct_invalid.cc - test input file for iwyu ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --silence_correct -I .

#include "tests/cxx/comment_style-d1.h"

/***** IWYU_SUMMARY
tests/cxx/silence_correct_invalid.cc should add these lines:

tests/cxx/silence_correct_invalid.cc should remove these lines:
- #include "tests/cxx/comment_style-d1.h"  // lines XX-XX

The full include-list for tests/cxx/silence_correct_invalid.cc:

****** IWYU_SUMMARY */

