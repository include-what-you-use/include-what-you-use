//===--- mapping_regex.cc - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . \
//            -Xiwyu --regex=ecmascript \
//            -Xiwyu --mapping_file=tests/cxx/mapping_regex.imp

// This test is a little forced, but here's the idea:
// * The include of tests/cxx/direct.h should nominally be replaced by
//   tests/cxx/indirect.h, where IndirectClass is defined
// * But we provide a mapping file with a negative lookahead assertion mapping
//   any indirect.h **except in not-tests/not-cxx** to foobar.h
// * Since tests/cxx/indirect.h does not match not-tests/not-cxx/indirect.h,
//   the mapping will apply...
// * ... but only since the --regex=ecmascript, which is the only dialect
//   supporting negative lookahead.

#include "tests/cxx/direct.h"

void f() {
  // IWYU: IndirectClass is...*foobar.h
  IndirectClass i;
}

/**** IWYU_SUMMARY

tests/cxx/mapping_regex.cc should add these lines:
#include "foobar.h"

tests/cxx/mapping_regex.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/mapping_regex.cc:
#include "foobar.h"  // for IndirectClass

***** IWYU_SUMMARY */
