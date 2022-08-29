//===--- enums_ms_mode.cc - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -fms-extensions

// Tests Microsoft-specific aspects of enumeration handling. In particular,
// tests that MSVC predefined variable __FUNCSIG__ triggers suggestions
// to provide complete enum definition if any enumeration item is present
// in the function template arguments.

#include "tests/cxx/enums_ms_mode-direct.h"

template <auto E>
auto GetLFunctionStr() {
  return L__FUNCTION__;
}

template <auto E>
auto GetFuncDNameStr() {
  return __FUNCDNAME__;
}

template <auto E>
auto GetFuncSigStr() {
  return __FUNCSIG__;
}

template <auto E>
auto GetLFuncSigStr() {
  return L__FUNCSIG__;
}

void Fn() {
  // IWYU: IndirectScopedEnum needs a declaration
  constexpr IndirectScopedEnum ie{};
  GetLFunctionStr<ie>();
  GetFuncDNameStr<ie>();
  // The complete enum definition is required to produce correct strings
  // containing the item name as written.
  // IWYU: IndirectScopedEnum is...*-indirect.h
  GetFuncSigStr<ie>();
  // IWYU: IndirectScopedEnum is...*-indirect.h
  GetLFuncSigStr<ie>();
}

/**** IWYU_SUMMARY

tests/cxx/enums_ms_mode.cc should add these lines:
#include "tests/cxx/enums_ms_mode-indirect.h"

tests/cxx/enums_ms_mode.cc should remove these lines:
- #include "tests/cxx/enums_ms_mode-direct.h"  // lines XX-XX

The full include-list for tests/cxx/enums_ms_mode.cc:
#include "tests/cxx/enums_ms_mode-indirect.h"  // for IndirectScopedEnum

***** IWYU_SUMMARY */
