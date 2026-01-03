//===--- 1810.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I tests/bugs/1810
// IWYU_XFAIL

#include <cstdio>
#include "usearray.h"

int main(int argc, char** argv) {
  UsesArray ua{};
  printf("%d\n", ua.theArray[5]);
  return 0;
}

/**** IWYU_SUMMARY

(tests/bugs/1810/1810.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
