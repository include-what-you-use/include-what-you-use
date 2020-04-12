//===--- operator_new.cc - test input file for iwyu -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Test that iwyu suggests the include for <new> be removed if only
// built-in functions are used.

#include <new>
#include "tests/cxx/direct.h"

// The most primitive ::operator new/delete are builtins, and are basically
// wrappers around malloc.
void ExplicitOperators() {
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  IndirectClass* elem = (IndirectClass*)::operator new(sizeof(IndirectClass));
  ::operator delete(elem);

  // IWYU: IndirectClass needs a declaration
  IndirectClass* arr =
      // IWYU: IndirectClass needs a declaration
      // IWYU: IndirectClass is...*indirect.h
      (IndirectClass*)::operator new[](4 * sizeof(IndirectClass));
  ::operator delete[](arr);
}

// New- and delete-expressions, unless using placement syntax, only use builtin
// operators. They're equivalent with the above, but also run ctors/dtors.
// For placement syntax, see tests/cxx/placement_new.cc
void ExpressionsBuiltinTypes() {
  char* elem = new char;
  delete elem;

  int* arr = new int[4];
  delete[] arr;
}

// New- and delete-expressions with user-defined types.
void ExpressionsUserTypes() {
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  IndirectClass* elem = new IndirectClass;
  // IWYU: IndirectClass is...*indirect.h
  delete elem;

  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  IndirectClass* arr = new IndirectClass[4];
  // IWYU: IndirectClass is...*indirect.h
  delete[] arr;
}

// Aligned allocation uses operator new(size_t, std::align_val_t) under the
// hood in C++17, but does not require <new> to be included for it. Pre-C++17,
// the alignment is silently ignored (or unsupported if the standard library
// does not support aligned allocation).
void ImplicitAlignedAllocation() {
  struct alignas(32) Aligned {
    float value[8];
  };

  Aligned* elem = new Aligned;
  delete elem;

  Aligned* arr = new Aligned[10];
  delete[] arr;
}

/**** IWYU_SUMMARY

tests/cxx/operator_new.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/operator_new.cc should remove these lines:
- #include <new>  // lines XX-XX
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/operator_new.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
