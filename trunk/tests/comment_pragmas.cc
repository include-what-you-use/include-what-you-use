//===--- comment_pragmas.cc - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests the pragma-like comment-commands that iwyu recognizes,
// as described at the top of iwyu_preprocessor.h.

#include "tests/comment_pragmas-d1.h"

// Due to the comments in i1.h, it will be reported as the provider of
// IndirectClass, even though the class is really defined in indirect.h.
// IWYU: IndirectClass is...*comment_pragmas-i1.h
IndirectClass b;

/**** IWYU_SUMMARY

tests/comment_pragmas.cc should add these lines:
#include "tests/comment_pragmas-i1.h"

tests/comment_pragmas.cc should remove these lines:
- #include "tests/comment_pragmas-d1.h"  // lines XX-XX

The full include-list for tests/comment_pragmas.cc:
#include "tests/comment_pragmas-i1.h"  // for IndirectClass

***** IWYU_SUMMARY */
