//===--- comment_pragmas-i6.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This is a private file included directly by comment_pragmas.cc which
// exports the private file d3.h.

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_COMMENT_PRAGMAS_I6_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_COMMENT_PRAGMAS_I6_H_

// IWYU pragma: begin_exports
#include "tests/comment_pragmas-d3.h"
// IWYU pragma: end_exports

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_COMMENT_PRAGMAS_I6_H_
