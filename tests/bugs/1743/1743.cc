//===--- 1743.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

enum Foo {
#include "enumerators.def"
};

/**** IWYU_SUMMARY

(tests/bugs/1743/1743.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
