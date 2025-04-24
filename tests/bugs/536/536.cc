//===--- 536.cc - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include "intent-decl.h"
#include "intent-type.h"

void func() {
  decl();  // Full-use of Type in return
}

/**** IWYU_SUMMARY

(tests/bugs/536/536.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
