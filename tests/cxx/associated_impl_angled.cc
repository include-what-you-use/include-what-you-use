//===--- associated_impl_angled.cc - test input file for iwyu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --mapping_file=tests/cxx/associated_impl_angled.imp -I .

// Tests that the delimiters in an 'associated_impl' mapping do not matter.
// The mapping uses angle brackets, while the files are found through -I and
// so are compared as quoted includes.

#include "tests/cxx/associated_impl_angled.h"

void Use() {
  AngledInlineClass c;
}

/**** IWYU_SUMMARY

(tests/cxx/associated_impl_angled.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
