//===--- comment_pragmas-d4.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This is a private file included directly by comment_pragmas.cc.

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_COMMENT_PRAGMAS_D4_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_COMMENT_PRAGMAS_D4_H_

// NOTE: comment_pragmas-i7.h includes but doesn't export this file.
// IWYU pragma: private, include "tests/comment_pragmas-i7.h"

class CommentPragmasD4 {};

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_COMMENT_PRAGMAS_D4_H_
