//===--- silence_correct_valid.cc - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --silence_correct -I .

#include "tests/cxx/comment_style-d1.h"

int main() {
  Foo::bar(1);
  return 0;
}

/**** IWYU_SUMMARY
***** IWYU_SUMMARY */
