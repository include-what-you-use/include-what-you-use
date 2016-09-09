//===--- comment_pragmas-d11.h - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This is a private file included directly by comment_pragmas.cc.

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_D11_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_D11_H_

// Note: there is no file no_such_file2.h. It isn't needed for this test.
// IWYU pragma: private, include "tests/cxx/no_such_file2.h"
// IWYU pragma: friend tests/cxx/comment_pragmas.cc
class CommentPragmasD11 {};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_D11_H_
