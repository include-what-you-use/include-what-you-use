//===--- 1843.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL
// IWYU_ARGS: -Wno-unused-value

#include "class.h"
#include "tpl.h"

void Fn(Tpl<Class>& r) {
  (void)&r;      // 1
  (void)(r, 0);  // 2
  Tpl<int> ti;
}

/**** IWYU_SUMMARY

(tests/bugs/1843/1843.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
