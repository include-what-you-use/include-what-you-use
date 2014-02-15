//===--- backwards_includes.cc - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This tests a few situations where iwyu sometimes attributes uses
// incorrectly.  We want to make sure we catch these errors in our
// sanity-checking phase and don't suggest ridiculous #includes as
// a result.
//
// Situation #1:
//   d1.h: class MyClass { class NestedClass; typedef NestedClass MyTypedef; };
//   d1-inl.h: class MyClass::NestedClass { ... }
// foo-inl.h should #include foo.h, but foo.h should *not* include foo-inl.h.
// iwyu might think it needs to, because of the typedef.
//
// Situation #2:
//   d1.h: #if MACRO ... #endif
//   d1-inl.h   #define MACRO 1  #include "foo.h"
// Again, foo.h should *not* #include foo-inl.h, even though iwyu might
// think it ought to to get the definition of MACRO.
//
// However, we want to make sure to still make proper suggestions for
// this case, which bears similarities to the above:
// Situation #3:
//   d2.h: class A {};
//   d3.h: A global_a;
//   d.cc: #include "d2.h" / #include "d3.h"

#include "tests/cxx/backwards_includes-d1-inl.h"
#include "tests/cxx/backwards_includes-d2.h"
#include "tests/cxx/backwards_includes-d3.h"

Dummy d;                     // Just so we use something from d1-inl.h

A a_copy = global_a;         // use something from -d2.h and -d3.h

/**** IWYU_SUMMARY

(tests/cxx/backwards_includes.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
