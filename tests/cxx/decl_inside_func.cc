//===--- decl_inside_func.cc - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// A declaration inside a function can sometimes escape (see implementation of
// MACRO), and while such a declaration may look like it's independently used
// here, the only way to make contact with it is through its containing
// function.
// Since we must have a declaration of _the function_ in order to call it, the
// contained declaration will have followed along and we can ignore any direct
// uses of the contained decl.
// This used to lead to the contained declaration being fwd-decl used, and IWYU
// crashing when trying to format a printable forward-decl. Ignoring it
// completely avoids that malfunction.

// IWYU_ARGS: -I . -Xiwyu --check_also="tests/cxx/decl_inside_func-i1.h"

#include "tests/cxx/decl_inside_func-d1.h"

const char* f() {
  // IWYU: MACRO is ...*decl_inside_func-i1.h
  return MACRO("bleh").value();
}

/**** IWYU_SUMMARY

tests/cxx/decl_inside_func.cc should add these lines:
#include "tests/cxx/decl_inside_func-i1.h"

tests/cxx/decl_inside_func.cc should remove these lines:
- #include "tests/cxx/decl_inside_func-d1.h"  // lines XX-XX

The full include-list for tests/cxx/decl_inside_func.cc:
#include "tests/cxx/decl_inside_func-i1.h"  // for MACRO

***** IWYU_SUMMARY */
