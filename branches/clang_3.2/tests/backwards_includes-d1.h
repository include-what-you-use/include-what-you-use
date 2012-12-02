//===--- backwards_includes-d1.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Situation #1:
//   d1.h: class MyClass { class NestedClass; typedef NestedClass MyTypedef; };
//   d1-inl.h: class MyClass::NestedClass { ... }
class MyClass {
  class NestedClass;
  typedef NestedClass MyTypedef;
};

// Situation #2:
//   d1.h: #if MACRO ... #endif
//   d1-inl.h   #define MACRO 1  #include "foo.h"
#if MACRO
const int kMacro = 1;
#endif

/**** IWYU_SUMMARY

(tests/backwards_includes-d1.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
