//===--- keep_embedded_include-d1.h - test input file for iwyu ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

Item1 =
#include "tests/cxx/keep_embedded_include-d3.h"
,
Item2,
Item3

/**** IWYU_SUMMARY

(tests/cxx/keep_embedded_include-d1.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
