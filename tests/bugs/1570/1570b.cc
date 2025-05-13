//===--- 1570b.cc - iwyu test ---------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL
#include "bar-fwd.h"  // Bar (forward)

// Note IWYU suggests to forward-declare Bar as
//
//    template <typename T = int> class Bar;
//
// which leads to errors down the line as default arguments must only be present
// on one decl.

// Forward should suffice.
Bar<>* p;

/**** IWYU_SUMMARY

(tests/bugs/1570/1570b.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
