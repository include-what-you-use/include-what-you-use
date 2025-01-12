//===--- comment_pragmas-d10.h - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This is a private file included directly by comment_pragmas.cc.

/** @file tests/cxx/comment_pragmas-d10.h
 *  This is an internal header file, included by other library headers
 *  Do not attempt to use it directly.
 *  @headername{some_public_header_file, some_other_public_header_file}
 */

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_D10_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_D10_H_

#include "tests/cxx/comment_pragmas-i9.h"

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_D10_H_
