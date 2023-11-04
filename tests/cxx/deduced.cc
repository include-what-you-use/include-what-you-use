//===--- deduced.cc - test input file for iwyu ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests deduced type handling.

#include "tests/cxx/direct.h"

// IWYU: IndirectClass needs a declaration
IndirectClass& GetRef();

// IWYU: IndirectClass needs a declaration
void Fn(IndirectClass& i) {
  auto&& fwd_ref = i;
  // IWYU: IndirectClass is...*indirect.h
  fwd_ref.Method();

  // IWYU: IndirectClass is...*indirect.h
  auto param_copy = i;
  auto& param_ref1 = i;
  decltype(auto) param_ref2 = i;

  // IWYU: IndirectClass is...*indirect.h
  auto value = GetRef();
  auto& ref1 = GetRef();
  decltype(auto) ref2 = GetRef();
}

/**** IWYU_SUMMARY

tests/cxx/deduced.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/deduced.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/deduced.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
