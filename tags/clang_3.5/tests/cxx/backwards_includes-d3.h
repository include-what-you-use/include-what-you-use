//===--- backwards_includes-d3.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Situation #3:
//   d2.h: class A {};
//   d3.h: A global_a;
//   d.cc: #include "d2.h" / #include "d3.h"

// IWYU: A is...*backwards_includes-d2.h
A global_a;

/**** IWYU_SUMMARY

tests/cxx/backwards_includes-d3.h should add these lines:
#include "tests/cxx/backwards_includes-d2.h"

tests/cxx/backwards_includes-d3.h should remove these lines:

The full include-list for tests/cxx/backwards_includes-d3.h:
#include "tests/cxx/backwards_includes-d2.h"  // for A

***** IWYU_SUMMARY */
