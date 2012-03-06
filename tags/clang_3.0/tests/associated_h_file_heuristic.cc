//===--- associated_h_file_heuristic.cc - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that we correctly say that a .h is an 'associated' .h file
// of a .cc file if it occurs first and shares a basename.

#include "tests/internal/associated_h_file_heuristic.h"
#include <stdio.h>
#include <time.h>

FILE* f = 0;

/**** IWYU_SUMMARY

tests/associated_h_file_heuristic.cc should add these lines:

tests/associated_h_file_heuristic.cc should remove these lines:
- #include <time.h>  // lines XX-XX

The full include-list for tests/associated_h_file_heuristic.cc:
#include "tests/internal/associated_h_file_heuristic.h"
#include <stdio.h>  // for FILE

***** IWYU_SUMMARY */
