//===--- comment_pragmas-d7.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This file has various faulty pragmas.

// IWYU: @headername directive missing a closing brace
/** @file comment_pragmas-d7.h @headername{missing_close_brace
*/

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_COMMENT_PRAGMAS_D7_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_COMMENT_PRAGMAS_D7_H_

// IWYU: end_exports without a begin_exports
// IWYU pragma: end_exports

// IWYU pragma: begin_exports
// IWYU: Expected end_exports pragma
#include "tests/indirect.h"  // IWYU pragma: keep
// IWYU pragma: end_exports

// IWYU: Unknown or malformed pragma \(foo\)
// IWYU pragma: foo

// IWYU: begin_exports without an end_exports
// IWYU pragma: begin_exports

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_COMMENT_PRAGMAS_D7_H_
