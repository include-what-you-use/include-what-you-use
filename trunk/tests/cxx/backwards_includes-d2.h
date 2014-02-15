//===--- backwards_includes-d2.h - test input file for iwyu ---------------===//
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

class A {};

/**** IWYU_SUMMARY

(tests/cxx/backwards_includes-d2.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
