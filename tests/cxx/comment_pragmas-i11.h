//===--- comment_pragmas-i11.h - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// A public header file exporting the symbol CommentPragmasI11.
#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_I11_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_I11_H_

/* IWYU pragma: private */
/* IWYU pragma: friend tests/cxx/.*-d16.h */

class CommentPragmasI11 {};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_I11_H_
