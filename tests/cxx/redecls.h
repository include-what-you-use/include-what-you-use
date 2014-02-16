//===--- redecls.h - test input file for iwyu -----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/redecls-d1.h"

// This looks suspiciously like redecls-d1.h and redecls-d2.h.  But with
// uses, of course.

typedef int MyTypedef;
void Fn();
extern float var;

// Put this after our decls to encourage clang to associate the uses
// below with the declarations in -d2.h, rather than this file.
#include "tests/cxx/redecls-d2.h"

void Uses() {
  MyTypedef mt;
  Fn();
  (void)var;
}

/**** IWYU_SUMMARY

tests/cxx/redecls.h should add these lines:

tests/cxx/redecls.h should remove these lines:
- #include "tests/cxx/redecls-d1.h"  // lines XX-XX
- #include "tests/cxx/redecls-d2.h"  // lines XX-XX

The full include-list for tests/cxx/redecls.h:

***** IWYU_SUMMARY */
