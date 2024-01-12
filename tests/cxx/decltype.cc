//===--- decltype.cc - test input file for iwyu ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests reporting types under 'decltype' specifier.

#include "tests/cxx/decltype-d1.h"
#include "tests/cxx/direct.h"

struct WithStatic {
  // IWYU: IndirectClass needs a declaration
  static IndirectClass obj;
  // IWYU: Tpl needs a declaration
  // IWYU: IndirectClass needs a declaration
  static Tpl<IndirectClass> tpl;
};

// IWYU: IndirectClass is...*indirect.h
decltype(WithStatic::obj) obj;
// IWYU: Tpl is...*decltype-i1.h
// IWYU: IndirectClass is...*indirect.h
decltype(WithStatic::tpl) tpl;

// IWYU: IndirectClass is...*indirect.h
class Derived : decltype(WithStatic::obj) {};

void Fn() {
  // IWYU: IndirectClass is...*indirect.h
  (void)&decltype(WithStatic::obj)::a;
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(decltype(WithStatic::obj));
  // No need of the full type in fwd-declarable context.
  decltype(WithStatic::obj)* p = nullptr;
}

/**** IWYU_SUMMARY

tests/cxx/decltype.cc should add these lines:
#include "tests/cxx/decltype-i1.h"
#include "tests/cxx/indirect.h"

tests/cxx/decltype.cc should remove these lines:
- #include "tests/cxx/decltype-d1.h"  // lines XX-XX
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/decltype.cc:
#include "tests/cxx/decltype-i1.h"  // for Tpl
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
