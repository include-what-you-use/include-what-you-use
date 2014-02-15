//===--- varargs_and_references.cc - test input file for iwyu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// When passing a class to a function that takes vararg arguments,
// compilers seem to require the full type information for the class
// (though the behavior is technically undefined).  Test that IWYU
// notices this.  (It only matters for references, of course.)
//
// TODO(csilvers): Clarify what the intended behavior here is; see discussion on
// OCL 23543695.

#include "tests/cxx/direct.h"

// Just to make things more complicated, put a default argument before
// the varargs.
int Function(int x, int y = 5, ...) { return 0; }

// To make things even *more* complicated, try a function pointer.
int (*function_p)(int, int, ...) = &Function;

int main() {
  // IWYU: IndirectClass is...*indirect.h
  IndirectClass ic;
  // IWYU: IndirectClass needs a declaration
  const IndirectClass& ref = ic;
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof(Function(1, 2, ref));
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof((*function_p)(1, 2, ref));
  // IWYU: IndirectClass is...*indirect.h
  (void)sizeof((*function_p)(1, 2, 3, 4, 5, ref));
}

/**** IWYU_SUMMARY

tests/cxx/varargs_and_references.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/varargs_and_references.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/varargs_and_references.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
