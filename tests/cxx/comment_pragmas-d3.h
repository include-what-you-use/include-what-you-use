//===--- comment_pragmas-d3.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This is a private file included directly by comment_pragmas.cc.

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_D3_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_D3_H_

// NOTE: comment_pragmas-i6.h explicitly exports this file.
// IWYU pragma: private, include "tests/cxx/comment_pragmas-i6.h"

class CommentPragmasD3 {};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_D3_H_
