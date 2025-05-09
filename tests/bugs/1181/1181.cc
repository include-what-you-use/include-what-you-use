//===--- 1181.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include "1181.h"
enum class color { RED = 2 };
enum_data<color> d;

/**** IWYU_SUMMARY

(tests/bugs/1181/1181.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
