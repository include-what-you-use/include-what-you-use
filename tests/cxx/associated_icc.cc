//===--- associated_icc.cc - test input file for iwyu ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --keep=tests/cxx/associated_icc.icc -Xiwyu --check_also=tests/cxx/associated_icc.icc -Xiwyu --mapping_file=tests/cxx/associated_icc.imp -I .

// Tests the 'associated_header' mapping directive.  associated_icc.h
// keeps its inline definitions in associated_icc.icc and #includes that
// file itself, so the .icc is analyzed as part of the .h and may use what
// the .h #includes.  This is the mirror image of the foo.cc/foo.h
// association.

#include "tests/cxx/associated_icc.h"

void Use() {
  AssociatedIccClass a;
  a.InlinedMethod();
}

/**** IWYU_SUMMARY

(tests/cxx/associated_icc.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
