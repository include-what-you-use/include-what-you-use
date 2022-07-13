//===--- pointer_arith.cc - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Pointer arithmetic requires the full type of the pointed-to type, because
// type size is material to the calculations.

#include "tests/cxx/direct.h"

// IWYU: IndirectClass is...*indirect.h
IndirectClass ic1, ic2;

void PointerArithmetic() {
  // IWYU: IndirectClass needs a declaration
  IndirectClass* p1 = &ic1;
  // IWYU: IndirectClass needs a declaration
  IndirectClass* p2 = &ic2;

  // All the pointer arithmetic below require the full type.

  // Pointer minus pointer (should really use ptrdiff_t, but I don't want to
  // have to include headers for it)
  // IWYU: IndirectClass is...*indirect.h
  long x = p2 - p1;

  // Pointer minus offset
  // IWYU: IndirectClass is...*indirect.h
  void* p3 = p1 - 20;

  // Pointer decrement
  // IWYU: IndirectClass is...*indirect.h
  p1 -= 10;

  // Pointer plus offset
  // IWYU: IndirectClass is...*indirect.h
  p3 = p1 + 100;

  // Pointer increment
  // IWYU: IndirectClass is...*indirect.h
  p1 += 100;
}

// IWYU: IndirectTemplate is...*indirect.h
// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
IndirectTemplate<IndirectClass> itc1, itc2;

void PointerArithmeticWithTemplates() {
  // IWYU: IndirectTemplate needs a declaration
  // IWYU: IndirectClass needs a declaration
  IndirectTemplate<IndirectClass>* p1 = &itc1;
  // IWYU: IndirectTemplate needs a declaration
  // IWYU: IndirectClass needs a declaration
  IndirectTemplate<IndirectClass>* p2 = &itc2;

  // All the pointer arithmetic below require the full type.

  // Pointer minus pointer
  // IWYU: IndirectTemplate is...*indirect.h
  // IWYU: IndirectClass is...*indirect.h
  long x = p2 - p1;

  // Pointer minus offset
  // IWYU: IndirectTemplate is...*indirect.h
  // IWYU: IndirectClass is...*indirect.h
  void* p3 = p1 - 20;

  // Pointer decrement
  // IWYU: IndirectTemplate is...*indirect.h
  // IWYU: IndirectClass is...*indirect.h
  p1 -= 10;

  // Pointer plus offset
  // IWYU: IndirectTemplate is...*indirect.h
  // IWYU: IndirectClass is...*indirect.h
  p3 = p1 + 100;

  // Pointer increment
  // IWYU: IndirectTemplate is...*indirect.h
  // IWYU: IndirectClass is...*indirect.h
  p1 += 100;
}

// Make sure pointer arithmetic with builtins does not yield IWYU warnings.
void BuiltinPointerArithmetic() {
  char c = 0;
  char* pc = &c;
  pc -= 10;
  pc += 100;
  long x = pc - &c;

  int i = 0;
  int* pi = &i;
  pi -= 20;
  pi += 200;
  x = pi - &i;
}

/**** IWYU_SUMMARY

tests/cxx/pointer_arith.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/pointer_arith.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/pointer_arith.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass, IndirectTemplate

***** IWYU_SUMMARY */
