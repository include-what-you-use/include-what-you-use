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

#include "tests/cxx/direct.h"

struct WithStatic {
  // IWYU: IndirectClass needs a declaration
  static IndirectClass obj;
  // IWYU: IndirectTemplate needs a declaration
  // IWYU: IndirectClass needs a declaration
  static IndirectTemplate<IndirectClass> tpl;
};

// TODO: IWYU: IndirectClass is...*indirect.h
decltype(WithStatic::obj) obj;
// TODO: IWYU: IndirectTemplate is...*indirect.h
// IWYU: IndirectClass is...*indirect.h
decltype(WithStatic::tpl) tpl;

/**** IWYU_SUMMARY

tests/cxx/decltype.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/decltype.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/decltype.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass, IndirectTemplate (ptr only)

***** IWYU_SUMMARY */
