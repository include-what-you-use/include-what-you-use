//===--- associated_pragma.h - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/indirect.h"

class PragmaInlineClass {
 public:
  void InlinedMethod();

  IndirectClass member;
};

#include "tests/cxx/associated_pragma.inline"  // IWYU pragma: associated_impl

/**** IWYU_SUMMARY

(tests/cxx/associated_pragma.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
