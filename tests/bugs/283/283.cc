//===--- 283.cc - test input file for iwyu --------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include "macro.h"
#include "function.h"

MACRO_A(mydomain)

/**** IWYU_SUMMARY

(tests/bugs/283/283.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
