//===--- no_def_arg_call_site.cc - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --check_also="tests/cxx/no_def_arg_call_site-d1.h" -I .

// Tests that IWYU doesn't report default function arguments when analyzing call
// site.

#include "tests/cxx/no_def_arg_call_site-d1.h"

template <typename T>
void TplFn() {
  T::FnWithDefArg();
}

void Fn() {
  TplFn<Struct>();
  Struct::FnWithDefArg();
}

/**** IWYU_SUMMARY

(tests/cxx/no_def_arg_call_site.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
