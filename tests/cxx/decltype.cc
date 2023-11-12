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

// TODO: IWYU: IndirectClass is...*indirect.h
decltype(WithStatic::obj) obj;
// TODO: IWYU: Tpl is...*decltype-i1.h
// IWYU: IndirectClass is...*indirect.h
decltype(WithStatic::tpl) tpl;

/**** IWYU_SUMMARY

tests/cxx/decltype.cc should add these lines:
#include "tests/cxx/indirect.h"
template <typename T> struct Tpl;

tests/cxx/decltype.cc should remove these lines:
- #include "tests/cxx/decltype-d1.h"  // lines XX-XX
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/decltype.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass
template <typename T> struct Tpl;

***** IWYU_SUMMARY */
