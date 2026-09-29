//===--- associated_icc_redundant_include.cc - test input file for iwyu ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --keep=tests/cxx/associated_icc_redundant_include.icc -Xiwyu --check_also=tests/cxx/associated_icc_redundant_include.icc -Xiwyu --mapping_file=tests/cxx/associated_icc_redundant_include.imp -I .

// Some codebases defensively #include foo.h from foo.icc as well, even
// though foo.icc is only ever reached through foo.h.  The mapping file
// names the .icc -> .h direction only, so that back-#include must not
// make the .h associated with the .icc too: two files that are each
// other's associated header crash IWYU.  The redundant #include is kept,
// not reported for removal.  See associated_mapping_cycle.cc for a
// mapping file that does name both directions.

#include "tests/cxx/associated_icc_redundant_include.h"

void Use() {
  RedundantIccClass r;
  r.InlinedMethod();
}

/**** IWYU_SUMMARY

(tests/cxx/associated_icc_redundant_include.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
