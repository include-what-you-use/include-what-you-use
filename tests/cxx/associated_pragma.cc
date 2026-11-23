//===--- associated_pragma.cc - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests the 'associated_impl' pragma, the per-#include form of the
// 'associated_impl' mapping directive.  No mapping file here.

#include "tests/cxx/associated_pragma.h"

void Use() {
  PragmaInlineClass c;
  c.InlinedMethod();
}

/**** IWYU_SUMMARY

(tests/cxx/associated_pragma.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
