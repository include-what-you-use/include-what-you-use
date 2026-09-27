//===--- spliced_include.cc - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests #include lines spliced over multiple lines (some overlap with
// computed_include.cc).

// #include keyword itself spliced.
#i\
nc\
lude\
  "tests/cxx/direct.h"

// Include-name spliced.
#include <tests/\
cxx/\
direct.h>

// Computed include spliced.
#define MY_HEADER "tests/cxx/direct.h"

#include\
  MY_HEADER

// Computed with function-like include spliced.
#define STRINGIZE(x) #x

#include \
  STRINGIZE(tests/cxx/direct.h)

// IWYU: IndirectClass is...*indirect.h
IndirectClass x;

/**** IWYU_SUMMARY

tests/cxx/spliced_include.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/spliced_include.cc should remove these lines:
- #include <tests/cxx/direct.h>  // lines XX-XX+2
- #include "tests/cxx/direct.h"  // lines XX-XX+3
- #include "tests/cxx/direct.h"  // lines XX-XX+1
- #include "tests/cxx/direct.h"  // lines XX-XX+1

The full include-list for tests/cxx/spliced_include.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
