//===--- comment_pragmas-d14.h - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// A public header file exporting the symbol CommentPragmasD14.
#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_D14_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_D14_H_

// IWYU pragma: private
// IWYU pragma: friend nobody

class CommentPragmasD14 {};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_D14_H_
