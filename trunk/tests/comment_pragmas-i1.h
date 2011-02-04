//===--- comment_pragmas-i1.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This file has no content itself, except a pragma-like comment
// saying it re-exports all the symbols in "indirect.h"

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_COMMENT_PRAGMAS_I1_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_COMMENT_PRAGMAS_I1_H_

// We add in another pragma so the 'real' one, for indirect, isn't first.
// IWYU pragma: export symbols from "tests/non-existent-file.h"
// IWYU pragma: export symbols from "tests/indirect.h"

#include "tests/indirect.h"

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_COMMENT_PRAGMAS_I1_H_
