//===--- mapping_replace_regex_llvm.cc - test input file for iwyu ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . \
//            -Xiwyu --regex=llvm \
//            -Xiwyu --mapping_file=tests/cxx/mapping_replace_regex_llvm.imp

// Generically map include paths to a different include directory:
// * The include of tests/cxx/direct.h should nominally be replaced by
//   tests/cxx/indirect.h, where IndirectClass is defined
// * But we provide a mapping adding a "foobar" prefix to any include under
//   "tests/cxx", resulting in suggesting foobar/tests/cxx/indirect.h.

#include "tests/cxx/direct.h"

void f() {
  // IWYU: IndirectClass is defined in "foobar/tests/cxx/indirect.h"
  IndirectClass i;
}

/**** IWYU_SUMMARY

tests/cxx/mapping_replace_regex_llvm.cc should add these lines:
#include "foobar/tests/cxx/indirect.h"

tests/cxx/mapping_replace_regex_llvm.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/mapping_replace_regex_llvm.cc:
#include "foobar/tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
