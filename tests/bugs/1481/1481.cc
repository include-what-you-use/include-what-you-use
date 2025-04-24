//===--- 1481.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL
#include <cstdlib>

// Jury's still out on whether sys/wait.h should be required here.
// <cstdlib> is documented to provide WIFEXITED.
#include <sys/wait.h>

void f(int res) {
  if (WIFEXITED(res)) {
    abort();
  }
}

/**** IWYU_SUMMARY

(tests/bugs/1481/1481.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
