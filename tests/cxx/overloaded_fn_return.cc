//===--- overloaded_fn_return.cc - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests reporting full use of function return type in instantiated template
// when the function call is unresolved at template definition point.

#include "tests/cxx/direct.h"

class IndirectClass;

IndirectClass OverloadedFn(IndirectClass*);
void OverloadedFn(float*);

template <typename T>
void TplFn() {
  OverloadedFn((T*)nullptr);
}

void Fn() {
  // IWYU: IndirectClass is...*indirect.h
  TplFn<IndirectClass>();
}

/**** IWYU_SUMMARY

tests/cxx/overloaded_fn_return.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/overloaded_fn_return.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX
- class IndirectClass;  // lines XX-XX

The full include-list for tests/cxx/overloaded_fn_return.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
