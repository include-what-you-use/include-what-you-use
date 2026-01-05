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

// IWYU: FnWithSmearedDefArgs3 is...*-i1.h
void FnWithSmearedDefArgs3(int = 0, int, int);
void FnWithDefArg(int);

template <typename T>
void TplFn() {
  T::FnWithDefArg();
}

void Fn() {
  TplFn<Struct>();
  Struct::FnWithDefArg();

  // IWYU: FnWithSmearedDefArgs is...*-i2.h
  FnWithSmearedDefArgs(1);
  FnWithSmearedDefArgs();

  // IWYU: FnWithSmearedDefArgs2 is...*-i2.h
  FnWithSmearedDefArgs2(1);
  // IWYU: FnWithSmearedDefArgs2 is...*-i1.h
  FnWithSmearedDefArgs2();

  // IWYU: FnWithSmearedDefArgs2 is...*-i2.h
  // IWYU: ns::FnWithSmearedDefArgs2 is...*-i3.h...*for using decl
  ns::FnWithSmearedDefArgs2(1);
  // IWYU: FnWithSmearedDefArgs2 is...*-i1.h
  // IWYU: ns::FnWithSmearedDefArgs2 is...*-i3.h...*for using decl
  ns::FnWithSmearedDefArgs2();

  // An appropriate redeclaration is present in this file.
  FnWithSmearedDefArgs3();
  FnWithSmearedDefArgs3(1);
  FnWithSmearedDefArgs3(1, 1);
  FnWithSmearedDefArgs3(1, 1, 1);

  // IWYU: FnWithDefArg is...*-i2.h
  FnWithDefArg();
  // An appropriate redeclaration is present in this file.
  FnWithDefArg(1);

  // IWYU: operator new is...*-i2.h
  new (1, 1) int;
  new (1) int;
}

/**** IWYU_SUMMARY

tests/cxx/fn_def_args.cc should add these lines:
#include "tests/cxx/fn_def_args-i1.h"
#include "tests/cxx/fn_def_args-i2.h"
#include "tests/cxx/fn_def_args-i3.h"

tests/cxx/fn_def_args.cc should remove these lines:

The full include-list for tests/cxx/fn_def_args.cc:
#include "tests/cxx/fn_def_args-d1.h"  // for FnWithSmearedDefArgs, Struct, operator new
#include "tests/cxx/fn_def_args-i1.h"  // for FnWithSmearedDefArgs2, FnWithSmearedDefArgs3
#include "tests/cxx/fn_def_args-i2.h"  // for FnWithDefArg, FnWithSmearedDefArgs, FnWithSmearedDefArgs2, operator new
#include "tests/cxx/fn_def_args-i3.h"  // for FnWithSmearedDefArgs2

***** IWYU_SUMMARY */
