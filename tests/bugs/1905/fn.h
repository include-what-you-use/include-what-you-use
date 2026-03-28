//===--- fn.h - iwyu test -------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "direct.h"

ns::Ret Fn1(ns::Arg);
decltype(r) Fn2(decltype(a));

/**** IWYU_SUMMARY

(tests/bugs/1905/fn.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
