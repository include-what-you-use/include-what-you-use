//===--- built_ins_new_included.cc - test input file for iwyu -------------===//
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

void foo() {
  char* ch = new char;
  delete ch;
  int* int_array = new int[10];
  delete[] int_array;
}

/**** IWYU_SUMMARY

tests/built_ins_new_included.cc should add these lines:

tests/built_ins_new_included.cc should remove these lines:
- #include <new>  // lines XX-XX

The full include-list for tests/built_ins_new_included.cc:

***** IWYU_SUMMARY */
