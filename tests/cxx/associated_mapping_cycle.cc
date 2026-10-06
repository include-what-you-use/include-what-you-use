//===--- associated_mapping_cycle.cc - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --keep=tests/cxx/associated_mapping_cycle.icc -Xiwyu --check_also=tests/cxx/associated_mapping_cycle.icc -Xiwyu --mapping_file=tests/cxx/associated_mapping_cycle.imp -I .

// Regression test for a crash.  The mapping file names the association in
// both directions, and this translation unit enters each file from the
// other: the .icc first, which #includes the .h, which #includes the .icc
// back.  Honoring both directions would make the two analyses depend on
// each other and trip "Assertion failed:
// desired_includes_have_been_calculated_".  Only the direction seen first
// is kept.
//
// The .icc has to come first here, so that the .h is first seen as
// #included by the .icc.
#include "tests/cxx/associated_mapping_cycle.icc"
#include "tests/cxx/associated_mapping_cycle.h"

void Use() {
  CycleClass c;
  (void)c;
}

/**** IWYU_SUMMARY

(tests/cxx/associated_mapping_cycle.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
