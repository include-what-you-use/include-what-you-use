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
//
// Here are the various cases we are testing.
//
// 1. symbol in private file => recommendation to include public file not seen.
//    cpd2 in d2.h which wants no_such_file.h.
// 2. symbol in private file => recommendation to include exporting public file.
//    cpd3 in d3.h, wants (exporting) i6.h.
// 3. symbol in private file => recommendation to include non-exporting file.
//    cpd4 in d4.h, wants (non-exporting) i7.h.
// 4. symbol in public file => keep include with "exports" pragma.
//    cpi2 in i2.h, exported by i1.h.
// 5. symbol in public file => keep include within
//    "begin_exports/end_exports" pragmas.
//    cpi3 in i3.h, exported by i1.h.
//    cpi4 in i4.h, exported by i1.h.
// 6. symbol in public file => don't keep (non-exporting) includer.
//    ic in indirect.h, included by i2.h.
// 7. unneeded include protected by a "keep" pragma.
//    d5.h is needlessly included.
// 8. symbol in public file included by a "keeping" includer =>
//    the public file should be added, but includer kept.
//    cpi8 in i8.h, included by d6.h.
// 9. begin_exports without end_exports => warning
//    d7.h.
// 10. end_exports without begin_exports => warning
//     d7.h.
// 11. pragma inside begin_exports/end_exports => warning
//     d7.h
// 12. Unknown pragma => warning
//     d7.h

// TODO(user): Tests where both the defining public file and an
// exporting public file are included, once it's clear what the policy
// should be.

#include "tests/comment_pragmas-d1.h"
#include "tests/comment_pragmas-d2.h"
#include "tests/comment_pragmas-d3.h"
#include "tests/comment_pragmas-d4.h"
#include "tests/comment_pragmas-d5.h"  // IWYU pragma: keep
#include "tests/comment_pragmas-d6.h"  // IWYU pragma: keep
#include "tests/comment_pragmas-d7.h"

// The following classes are all defined in public files exported by i2.h.
// IWYU: CommentPragmasI2 is...*comment_pragmas-i1.h
CommentPragmasI2 cpi2;
// IWYU: CommentPragmasI3 is...*comment_pragmas-i1.h
CommentPragmasI3 cpi3;
// IWYU: CommentPragmasI4 is...*comment_pragmas-i1.h
CommentPragmasI4 cpi4;

// d2.h is a private file which says to include no_such_file.h.
// We haven't provided that file, because it's not needed for this test.
// IWYU: CommentPragmasD2 is...*no_such_file.h
CommentPragmasD2 cpd2;

// d3.h is a private file which says to include i6.h, which
// re-exports it.
// IWYU: CommentPragmasD3 is...*comment_pragmas-i6.h
CommentPragmasD3 cpd3;

// d4.h is a private file which says to include i7.h, which
// doesn't explicitly re-export it.
// IWYU: CommentPragmasD4 is...*comment_pragmas-i7.h
CommentPragmasD4 cpd4;

// d6.h includes i8.h and d6.h is "kept".
// IWYU: CommentPragmasI8 is...*comment_pragmas-i8.h
CommentPragmasI8 cpi8;

// indirect.h is not private and not exported by i2.h.
// IWYU: IndirectClass is ...*indirect.h
IndirectClass ic;

/**** IWYU_SUMMARY

tests/comment_pragmas.cc should add these lines:
#include "tests/comment_pragmas-i1.h"
#include "tests/comment_pragmas-i6.h"
#include "tests/comment_pragmas-i7.h"
#include "tests/comment_pragmas-i8.h"
#include "tests/indirect.h"
#include "tests/no_such_file.h"

tests/comment_pragmas.cc should remove these lines:
- #include "tests/comment_pragmas-d1.h"  // lines XX-XX
- #include "tests/comment_pragmas-d2.h"  // lines XX-XX
- #include "tests/comment_pragmas-d3.h"  // lines XX-XX
- #include "tests/comment_pragmas-d4.h"  // lines XX-XX
- #include "tests/comment_pragmas-d7.h"  // lines XX-XX

The full include-list for tests/comment_pragmas.cc:
#include "tests/comment_pragmas-d5.h"
#include "tests/comment_pragmas-d6.h"
#include "tests/comment_pragmas-i1.h"  // for CommentPragmasI2, CommentPragmasI3, CommentPragmasI4
#include "tests/comment_pragmas-i6.h"  // for CommentPragmasD3
#include "tests/comment_pragmas-i7.h"  // for CommentPragmasD4
#include "tests/comment_pragmas-i8.h"  // for CommentPragmasI8
#include "tests/indirect.h"  // for IndirectClass
#include "tests/no_such_file.h"  // for CommentPragmasD2

***** IWYU_SUMMARY */
