//===--- associated_icc_default.cc - test input file for iwyu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --keep=tests/cxx/associated_icc_default.icc -Xiwyu --check_also=tests/cxx/associated_icc_default.icc -I .

// Same as associated_icc.cc, but with no mapping file: .icc/.inl/.ipp/.tcc
// next to a header of the same stem are associated with it out of the box.

#include "tests/cxx/associated_icc_default.h"

void Use() {
  DefaultIccClass a;
  a.InlinedMethod();
}

/**** IWYU_SUMMARY

(tests/cxx/associated_icc_default.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
