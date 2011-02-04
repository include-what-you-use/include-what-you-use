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

#include "tests/direct.h"

// Just to make things more complicated, put a default argument before
// the varargs.
void Function(int x, int y = 5, ...) { }

// To make things even *more* complicated, try a function pointer.
void (*function_p)(int, int, ...) = &Function;

int main() {
  // IWYU: IndirectClass is...*indirect.h
  IndirectClass ic;
  // IWYU: IndirectClass needs a declaration
  const IndirectClass& ref = ic;
  // IWYU: IndirectClass is...*indirect.h
  Function(1, 2, ref);
  // IWYU: IndirectClass is...*indirect.h
  (*function_p)(1, 2, ref);
}

/**** IWYU_SUMMARY

tests/varargs_and_references.cc should add these lines:
#include "tests/indirect.h"

tests/varargs_and_references.cc should remove these lines:
- #include "tests/direct.h"  // lines XX-XX

The full include-list for tests/varargs_and_references.cc:
#include "tests/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
