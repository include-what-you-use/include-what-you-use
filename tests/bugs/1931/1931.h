//===--- 1931.h - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

// IWYU falsely suggests removing this header,
// which makes compilation fail.
#include "print.h"

struct A { void print(); };

template <>
void print(A) = delete;

/**** IWYU_SUMMARY

(tests/bugs/1931/1931.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
