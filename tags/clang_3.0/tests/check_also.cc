//===--- check_also.cc - test input file for iwyu -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests the '--check_also' flag.

#include "check_also-d1.h"   // part of the --check-also glob
#include "check_also-n1.h"   // not part of the --check-also glob

int main() {
  // IWYU: kI1 is...*check_also-i1.h
  return kI1;
}

/**** IWYU_SUMMARY

tests/check_also.cc should add these lines:
#include "tests/check_also-i1.h"

tests/check_also.cc should remove these lines:
- #include "check_also-d1.h"  // lines XX-XX
- #include "check_also-n1.h"  // lines XX-XX

The full include-list for tests/check_also.cc:
#include "tests/check_also-i1.h"  // for kI1

***** IWYU_SUMMARY */
