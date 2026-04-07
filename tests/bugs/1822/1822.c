//===--- 1822.c - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include "shadow.h"
#include "orig.h"

void foo(void) {
  call_orig();
}

/**** IWYU_SUMMARY

(tests/bugs/1822/1822.c has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
