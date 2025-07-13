//===--- 1774.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include "fn.h"

int main() {
  auto& class_ref = GetClass();
  auto& tpl_ref = GetTpl();
  [&class_ref, &tpl_ref] {}();
}

/**** IWYU_SUMMARY

(tests/bugs/1774/1774.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
