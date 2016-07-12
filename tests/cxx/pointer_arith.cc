//===--- pointer_arith.cc - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

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
  unsigned long x = p2 - p1;

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


/**** IWYU_SUMMARY

tests/cxx/pointer_arith.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/pointer_arith.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/pointer_arith.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
