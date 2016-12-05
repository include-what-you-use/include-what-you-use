//===--- comment_pragmas-d7.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This file has various faulty pragmas.

/** @file tests/cxx/comment_pragmas-d7.h
 */
// IWYU: @headername directive missing a closing brace
/*  @headername{missing_close_brace
*/

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_D7_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_D7_H_

// IWYU: end_exports without a begin_exports
// IWYU pragma: end_exports

// IWYU pragma: begin_exports
// IWYU: Expected end_exports pragma
#include "tests/cxx/indirect.h"  // IWYU pragma: keep
// IWYU pragma: end_exports

// IWYU: Unknown or malformed pragma \(foo\)
// IWYU pragma: foo

// IWYU: Suggested include must be a quoted header
// IWYU pragma: private, include not-a-quoted-header.h

// IWYU: Inhibited include must be a quoted header
// IWYU pragma: no_include not-a-quoted-header.h

// IWYU: begin_exports without an end_exports
// IWYU pragma: begin_exports

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_D7_H_
