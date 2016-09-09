//===--- comment_pragmas-i1.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_I1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_I1_H_

// Verify that pragma that don't start comments are ignored.
/*
// IWYU pragma: private, include "foo"  // This should be ignored.
*/
// // IWYU pragma: private, include "bar"
const char kIgnoreThis[] = "// IWYU pragma: private, include \"baz\"";

// And pragmas in uncompiled #ifs.
#if 0
// IWYU pragma: private, include "quz"
#endif

// Include a private file that declares us as the file to include.
#include "tests/cxx/comment_pragmas-i2.h"

// Re-export some files.
#include "tests/cxx/comment_pragmas-i3.h"  // IWYU pragma: export
// This comment is here to make sure that clang calls HandleComment
// once per whole-line comment.
// IWYU pragma: begin_exports
#include "tests/cxx/comment_pragmas-i4.h"
#include "tests/cxx/comment_pragmas-i5.h"
// IWYU pragma: end_exports

// Include a file after 'end_exports' that we don't re-export.
#include "tests/cxx/indirect.h"

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_COMMENT_PRAGMAS_I1_H_
