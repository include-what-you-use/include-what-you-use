//===--- enum_base.cc - test input file for iwyu --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/enum_base-d1.h"

// IWYU: int_t is...*tests/cxx/enum_base-i1.h
enum class Test : int_t {
    VALUE1,
    VALUE2
};

/**** IWYU_SUMMARY

tests/cxx/enum_base.cc should add these lines:
#include "tests/cxx/enum_base-i1.h"

tests/cxx/enum_base.cc should remove these lines:
- #include "tests/cxx/enum_base-d1.h"  // lines XX-XX

The full include-list for tests/cxx/enum_base.cc:
#include "tests/cxx/enum_base-i1.h"  // for int_t

***** IWYU_SUMMARY */
