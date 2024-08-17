//===--- comment_pragmas-d23.h - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Test libstdc++'s file directive with empty filename. We should map correctly
// even if filename is missing.

/** @file
 *  This is an internal header file, included by other library headers
 *  Do not attempt to use it directly. @headername{some_system_header_file}
 */

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_D23_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_D23_H_

class CommentPragmasD23 {};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_D23_H_
