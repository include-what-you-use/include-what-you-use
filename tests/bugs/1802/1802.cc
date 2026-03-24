//===--- 1802.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include "tpl.h"

void Fn() {
  Derived::Type t1;  // IWYU suggests Class
  Derived d;
  d.GetClass();  // IWYU suggests Class

  // In contrast, the type provision logic works fine for Alias, and IWYU
  // doesn't suggest Class.
  Alias::Type t2;
  Alias a;
  a.GetClass();
}

/**** IWYU_SUMMARY

(tests/bugs/1802/1802.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
