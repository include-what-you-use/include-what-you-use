//===--- update_comments.cc - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --update_comments -I .

// Test that passing the --update_comments switch to IWYU makes it always print
// the full include-list, with up to date "// for XYZ" comments.

#include "tests/cxx/indirect.h"  // for SomethingElse

IndirectClass indirect;

/**** IWYU_SUMMARY

tests/cxx/update_comments.cc should add these lines:

tests/cxx/update_comments.cc should remove these lines:

The full include-list for tests/cxx/update_comments.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
