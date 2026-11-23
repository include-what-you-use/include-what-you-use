//===--- associated_impl_angled.h - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/indirect.h"

class AngledInlineClass {
 public:
  void InlinedMethod();

  IndirectClass member;
};

#include "tests/cxx/associated_impl_angled-impl.inline"

/**** IWYU_SUMMARY

(tests/cxx/associated_impl_angled.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
