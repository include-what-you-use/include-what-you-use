//===--- expl_inst_macro.cc - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/expl_inst_macro.h"

// 'macro' expands to a use of 'std::cout'. Under gcc/libstd++ this triggers use
// of the explicit instantiation basic_ostream<char>.
// IWYU's explicit instantiation processing attempts to use source locations to
// find all redecls before in the translation unit. The fact that we're in a
// macro sometimes causes IWYU to fail to compute valid source locations, so the
// location arithmetic would cause an assertion failure.
// This opaque test case checks that we no longer do, in this specific case.

// IWYU_ARGS: -I .

void test() {
  macro;
}

/**** IWYU_SUMMARY

(tests/cxx/expl_inst_macro.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
