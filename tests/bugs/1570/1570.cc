//===--- 1570.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

// IWYU does not understand forwarding headers.

#include "foo-fwd.h"         // Foo (forward)

// Forward suffices.
Foo *p;

/**** IWYU_SUMMARY

(tests/bugs/1570/1570.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
