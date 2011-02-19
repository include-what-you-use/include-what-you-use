//===--- comment_pragmas-i1.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_COMMENT_PRAGMAS_I1_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_COMMENT_PRAGMAS_I1_H_

// Include a private file that declares us as the file to include.
#include "tests/comment_pragmas-i2.h"

// Re-export some files.
#include "tests/comment_pragmas-i3.h"  // IWYU pragma: export
// IWYU pragma: begin_exports
#include "tests/comment_pragmas-i4.h"
#include "tests/comment_pragmas-i5.h"
// IWYU pragma: end_exports

// Include a file after 'end_exports' that we don't re-export.
#include "tests/indirect.h"

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_COMMENT_PRAGMAS_I1_H_
