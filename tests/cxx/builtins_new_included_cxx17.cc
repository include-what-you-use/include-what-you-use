//===--- builtins_new_included_cxx17.cc - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Test that iwyu suggests the include for <new> be removed if only
// built-in functions are used, C++17 edition (i.e. with aligned allocations).

#include <new>

void foo() {
  class alignas(32) test_struct {
    float value[8];
  };

  test_struct* t = new test_struct;
  delete t;
  test_struct* t_array = new test_struct[10];
  delete[] t_array;
}

/**** IWYU_SUMMARY

tests/cxx/builtins_new_included_cxx17.cc should add these lines:

tests/cxx/builtins_new_included_cxx17.cc should remove these lines:
- #include <new>  // lines XX-XX

The full include-list for tests/cxx/builtins_new_included_cxx17.cc:

***** IWYU_SUMMARY */
