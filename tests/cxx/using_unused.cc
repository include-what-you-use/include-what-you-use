//===--- using_unused.cc - test input file for iwyu -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that when we import a name via a using statement, IWYU tells us we
// need to include the original thing we're referencing, despite the fact that
// it's not actually used.

#include "using_unused-declare.h"
#include "using_unused-baseclass.h"

using ns::symbol;

class Hiding : public Base {
  // Introduce a using shadow decl which is subsequently discarded when this
  // class hides the using-decl with its own declaration.
  // This used to trigger an assertion failure.
  using Base::Method;

  void Method();
};

/**** IWYU_SUMMARY

(tests/cxx/using_unused.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
