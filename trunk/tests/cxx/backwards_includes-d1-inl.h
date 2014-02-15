//===--- backwards_includes-d1-inl.h - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Situation #2:
//   d1.h: #if MACRO ... #endif
//   d1-inl.h   #define MACRO 1  #include "foo.h"
#define MACRO 1

#include "tests/cxx/backwards_includes-d1.h"

// Situation #1:
//   d1.h: class MyClass { class NestedClass; typedef NestedClass MyTypedef; };
//   d1-inl.h: class MyClass::NestedClass { ... }
class MyClass::NestedClass { };

class Dummy { };

/**** IWYU_SUMMARY

(tests/cxx/backwards_includes-d1-inl.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
