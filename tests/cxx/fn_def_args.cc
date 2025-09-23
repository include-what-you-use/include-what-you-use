//===--- fn_def_args.cc - test input file for iwyu ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --check_also="tests/cxx/fn_def_args-d1.h" -I .

// Tests handling function default arguments. In particular, tests that IWYU
// doesn't report them when analyzing call site.

#include "tests/cxx/fn_def_args-d1.h"

template <typename T>
void TplFn() {
  T::FnWithDefArg();
}

void Fn() {
  TplFn<Struct>();
  Struct::FnWithDefArg();
}

/**** IWYU_SUMMARY

(tests/cxx/fn_def_args.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
