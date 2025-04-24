//===--- 1428.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --mapping_file=tests/bugs/1428/m.imp
// IWYU_XFAIL

#include <time.h>

void f() {
  (void)time(nullptr);
}

/**** IWYU_SUMMARY

tests/bugs/1370/1370.c should add these lines:
#include <ctime>

tests/bugs/1370/1370.c should remove these lines:
#include <time.h>  // lines XX-XX

The full include-list for tests/bugs/1370/1370.c:
#include <ctime>  // for time

***** IWYU_SUMMARY */
