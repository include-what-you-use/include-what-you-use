//===--- fn_return_reexport.cc - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -Xiwyu --check_also=tests/cxx/*-direct.h

// Tests re-export policy for function return types.

#include "tests/cxx/fn_return_reexport-direct.h"

auto GetDeducedIndirectClass() {
  return GetProvidedIndirectClass();
}

auto get_indirect_class_lambda = [] { return GetProvidedIndirectClass(); };

// IWYU: IndirectClass needs a declaration
auto GetTrailingReturnType() -> IndirectClass;

template <typename T>
auto GetTrailingReturnTypeTpl() -> T;

void Test() {
  // There is no need to report IndirectClass here: it should be available
  // at the point of GetDeducedIndirectClass() and get_indirect_class_lambda
  // definitions, otherwise compilers could not deduce the return type.
  GetDeducedIndirectClass();
  get_indirect_class_lambda();
  // 'auto' with a trailing return type should not confuse IWYU: a user is
  // still responsible for the type info.
  // IWYU: IndirectClass is...*indirect.h
  GetTrailingReturnType();
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  GetTrailingReturnTypeTpl<IndirectClass>();
}

/**** IWYU_SUMMARY

tests/cxx/fn_return_reexport.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/fn_return_reexport.cc should remove these lines:

The full include-list for tests/cxx/fn_return_reexport.cc:
#include "tests/cxx/fn_return_reexport-direct.h"  // for GetProvidedIndirectClass
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
