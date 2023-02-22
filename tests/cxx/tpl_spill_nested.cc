//===--- tpl_spill_nested.cc - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/tpl_spill_nested-d1.h"

// IWYU_ARGS: -I .

void Instantiate() {
  // No diagnostic expected.
  Outer<int> o;

  // No diagnostic expected.
  o[100] = 1;
}

/**** IWYU_SUMMARY

(tests/cxx/tpl_spill_nested.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
