//===--- fn_def_args-d1.h - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/fn_def_args-i1.h"
#include "tests/cxx/fn_def_args-i3.h"
#include <cstddef>

struct Struct {
  // IWYU: GetInt() is...*fn_def_args-i2.h
  static void FnWithDefArg(int = GetInt());
};

// IWYU: FnWithSmearedDefArgs(int, int) is...*fn_def_args-i2.h
void FnWithSmearedDefArgs(int = 0, int);

// No need to report any redeclaration here because no default argument
// is added.
void FnWithSmearedDefArgs(int, int);
void FnWithDefArg2(int, int);

// IWYU: operator new(unsigned long, int, int, int) is...*fn_def_args-i2.h
void* operator new(std::size_t, int, int = 0, int);

/**** IWYU_SUMMARY

tests/cxx/fn_def_args-d1.h should add these lines:
#include "tests/cxx/fn_def_args-i2.h"

tests/cxx/fn_def_args-d1.h should remove these lines:
- #include "tests/cxx/fn_def_args-i1.h"  // lines XX-XX
- #include "tests/cxx/fn_def_args-i3.h"  // lines XX-XX

The full include-list for tests/cxx/fn_def_args-d1.h:
#include <cstddef>  // for size_t
#include "tests/cxx/fn_def_args-i2.h"  // for FnWithSmearedDefArgs, GetInt, operator new

***** IWYU_SUMMARY */
