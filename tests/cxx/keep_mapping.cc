//===--- keep_mapping.cc - test input file for iwyu -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --check_also="tests/cxx/*-public.h" \
//            -Xiwyu --mapping_file=tests/cxx/keep_mapping.imp \
//            -I .

// The real test here is in keep_mapping-public.h.

#include "tests/cxx/keep_mapping-public.h"

const int kCcInt = kInt;

/**** IWYU_SUMMARY

(tests/cxx/keep_mapping.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
