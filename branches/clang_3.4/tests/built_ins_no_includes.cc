//===--- built_ins_no_includes.cc - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Test that iwyu doesn't recommend anything for built-in functions
// when <new> is not included.

void foo() {
  char* ch = new char;
  delete ch;
  int* int_array = new int[10];
  delete[] int_array;
}

/**** IWYU_SUMMARY

(tests/built_ins_no_includes.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
