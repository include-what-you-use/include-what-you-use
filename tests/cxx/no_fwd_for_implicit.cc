//===--- no_fwd_for_implicit.cc - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests that IWYU does not suggest adding a forward-declaration when the type
// is not explicitly written in the source.

#include "tests/cxx/no_fwd_for_implicit-direct.h"

int main() {
  auto& class_ref = GetClass();
  auto& tpl_ref = GetTpl();
  // IWYU should report fwd-decl uses neither of Class nor of Tpl.
  [&class_ref, &tpl_ref] {}();
}

/**** IWYU_SUMMARY

(tests/cxx/no_fwd_for_implicit.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
