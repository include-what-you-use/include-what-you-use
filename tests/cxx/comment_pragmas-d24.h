//===--- comment_pragmas-d24.h - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Test libstdc++'s file directive with leading 'include/'. We should map
// correctly even if filename is strictly wrong.

/** @file include/tests/cxx/comment_pragmas-d24.h
 *  This is an internal header file, included by other library headers
 *  Do not attempt to use it directly. @headername{some_system_header_file}
 */

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_D24_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_D24_H_

class CommentPragmasD24 {};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_D24_H_
