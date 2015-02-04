//===--- catch.cc - test input file for iwyu ------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/catch-byvalue.h"  // for CatchByValue
#include "tests/cxx/catch-byref.h"    // for CatchByRef
#include "tests/cxx/catch-byptr.h"    // for CatchByPtr

int main() {
  try {
  } catch (const CatchByValue) {
  }

  try {
  } catch (const CatchByRef&) {
  }

  try {
  } catch (const CatchByPtr*) {
  }

  // Make sure we don't crash when there's no type.
  try {
  } catch (...) {
  }

  return 0;
}

/**** IWYU_SUMMARY

(tests/cxx/catch.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
