//===--- comment_pragmas-i11.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// A public header file exporting the symbol CommentPragmasI11.
#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_COMMENT_PRAGMAS_I11_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_COMMENT_PRAGMAS_I11_H_

/* IWYU pragma: private */
/* IWYU pragma: friend tests/.*-d16.h */

class CommentPragmasI11 {};

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_COMMENT_PRAGMAS_I11_H_
