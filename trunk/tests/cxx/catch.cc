//===--- catch.cc - test input file for iwyu ------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/catch-exceptions.h"

int main() {
  try {
  // IWYU: CatchByValue...*catch-byvalue.h
  } catch (const CatchByValue cbv) {
    // IWYU: LogException...*catch-logex.h
    LogException(cbv);
  }

  try {
  // IWYU: CatchByRef needs a declaration...*
  // IWYU: CatchByRef...*catch-byref.h
  } catch (const CatchByRef& cbr) {
    // IWYU: LogException...*catch-logex.h
    LogException(cbr);
  }

  try {
  // IWYU: CatchByPtr needs a declaration...*
  // IWYU: CatchByPtr...*catch-byptr.h
  } catch (const CatchByPtr* cpr) {
    // IWYU: LogException...*catch-logex.h
    LogException(*cpr);
  }

  // Make sure we see through elaborated types
  try {
  // IWYU: CatchElab needs a declaration...*
  // IWYU: CatchElab...*catch-elab.h
  } catch (const Namespace::CatchElab&) {
  }

  // Make sure we don't crash when there's no type.
  try {
    // IWYU: Thrown...*catch-thrown.h
    throw Thrown();
  } catch (...) {
    // IWYU: puts...*stdio.h
    puts("Unknown exception");
  }

  return 0;
}

/**** IWYU_SUMMARY

tests/cxx/catch.cc should add these lines:
#include <stdio.h>
#include "tests/cxx/catch-byptr.h"
#include "tests/cxx/catch-byref.h"
#include "tests/cxx/catch-byvalue.h"
#include "tests/cxx/catch-elab.h"
#include "tests/cxx/catch-logex.h"
#include "tests/cxx/catch-thrown.h"

tests/cxx/catch.cc should remove these lines:
- #include "tests/cxx/catch-exceptions.h"  // lines XX-XX

The full include-list for tests/cxx/catch.cc:
#include <stdio.h>  // for puts
#include "tests/cxx/catch-byptr.h"  // for CatchByPtr
#include "tests/cxx/catch-byref.h"  // for CatchByRef
#include "tests/cxx/catch-byvalue.h"  // for CatchByValue
#include "tests/cxx/catch-elab.h"  // for CatchElab
#include "tests/cxx/catch-logex.h"  // for LogException
#include "tests/cxx/catch-thrown.h"  // for Thrown

***** IWYU_SUMMARY */
