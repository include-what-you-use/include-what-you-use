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
// 13. @headername{foo} directive (gcc and ?) => include <foo>.
//     cp8 defined in d8.h which has @headername{some_system_header_file}
// 14. @headername{foo, bar} directive (gcc and ?) => include <foo>.
//     cp9 defined in d9.h which has
//     @headername{some_system_header_file, some_other_header_file}
// 15. Malformed @headername -> warning
//     d7.h
// 16. "no_include" pragma: Don't suggest include.
//     cpi9's type is defined in i9.h, included by d10.h.
//     d10.h will be deemed unnecessary, which is ok, and i9.h will
//     not be suggested.
// TODO(dsturtevant): Tests where both the defining public file and an
// exporting public file are included, once it's clear what the policy
// should be.
// 17. "friend" pragma: cpd11's type is defined in d11.h which is private
//     but declares this file as a friend.
// 18. "keep" keeps a "private" file: cpd17's type is defined in d17.h
//     which is private but this file declares it "keep". d17.h is still
//     included, and its suggested file is not.
// TODO(dsturtevant): More tests of "friend": globs, header files,
// quoted globs, more than one friend pragma in one file, file with more
// than one file befriending it.
// 19. "no_forward_declare" pragma: cpd18 is used in a way that can
//     be forward declared, but that forward declare is inhibited.
// 20. "no_forward_declare" pragma: cpd19 is used in a forward-declarable
//      way, but is forward declared anyway even though inhibited.
// 21. "no_forward_declare" pragma: Test21a is defined after a typedef,
//     which requires a forward declaration. This case is different because
//     internally IWYU wants a full-use which it downgrades to a forward-decl.
// 22. "no_forward_declare" pragma: cpd20a and cpd20b are defined inside an
//     anonymous namespace.
#include "tests/cxx/comment_pragmas-d1.h"
#include "tests/cxx/comment_pragmas-d10.h"
#include "tests/cxx/comment_pragmas-d11.h"
#include "tests/cxx/comment_pragmas-d12.h"
#include "tests/cxx/comment_pragmas-d13.h"
#include "tests/cxx/comment_pragmas-d14.h"
#include "tests/cxx/comment_pragmas-d15.h" /* IWYU pragma: keep */ /* check C-style comments */
#include "tests/cxx/comment_pragmas-d16.h"
#include "tests/cxx/comment_pragmas-d17.h"  // IWYU pragma: keep
#include "tests/cxx/comment_pragmas-d18.h"
#include "tests/cxx/comment_pragmas-d19.h"
#include "tests/cxx/comment_pragmas-d2.h"
#include "tests/cxx/comment_pragmas-d20.h"
#include "tests/cxx/comment_pragmas-d3.h"
#include "tests/cxx/comment_pragmas-d4.h"
#include "tests/cxx/comment_pragmas-d5.h"  // IWYU pragma: keep
#include "tests/cxx/comment_pragmas-d6.h"  // IWYU pragma: keep  // second instance, testing comment at end
#include "tests/cxx/comment_pragmas-d7.h"
#include "tests/cxx/comment_pragmas-d8.h"
#include "tests/cxx/comment_pragmas-d9.h"
// IWYU pragma: no_include "tests/cxx/comment_pragmas-i9.h"  // another test of comments
// IWYU pragma: no_include "tests/cxx/no_such_file_d17.h"
// IWYU pragma: no_forward_declare CommentPragmasD18
// IWYU pragma: no_forward_declare CommentPragmasD19
// IWYU pragma: no_forward_declare CommentPragmasD20a
// IWYU pragma: no_forward_declare Foo::CommentPragmasD20b
// IWYU pragma: no_forward_declare ::CommentPragmasD20c
// IWYU pragma: no_forward_declare CommentPragmasTest21a

// Keep all includes of any header name marked with pragma keep.
#include "tests/cxx/comment_pragmas-d21.h"
#include "tests/cxx/comment_pragmas-d21.h" // IWYU pragma: keep

#include "tests/cxx/comment_pragmas-d22.h" // IWYU pragma: keep
#include "tests/cxx/comment_pragmas-d22.h"

class CommentPragmasD19;  // Needed, but removed due to no_forward_declare.
class CommentPragmasTest21a;  // Needed but removed due to no_forward_declare.

class ForwardDeclaredUnnecessary1;  // IWYU pragma: keep
class ForwardDeclaredUnnecessary2;  /* IWYU pragma: keep */

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

// IWYU: CommentPragmasD8 is...*<some_system_header_file>
CommentPragmasD8 cpd8;

// IWYU: CommentPragmasD9 is...*<some_system_header_file>
CommentPragmasD9 cpd9;

// Note: IWYU will emit the diagnostic but suppress the include
// recommendation due to the no_include pragma.
// IWYU: CommentPragmasI9 is...*comment_pragmas-i9.h
CommentPragmasI9 cpi9;

// d11.h is a private file which says to include no_such_file2.h, but
// has a pragma saying it's ok for comment_pragmas.cc to include
// it.
CommentPragmasD11 cpd11;

// d11.h is a private file with no preferred includer.  It has a
// pragma saying it's ok for comment_pragmas.cc to include it.
CommentPragmasD12 cpd12;

// i10.h is included by d13.h. There's a pragma in i10.h declaring
// .*-d13.h friends.
CommentPragmasI10 cpi10;

// i11.h is included by d16.h. There's a c-style pragma in i11.h declaring
// .*-d16.h friends.
CommentPragmasI11 cpi11;

// d14.h is a private file with friend "nobody" and no suggested includes.
// IWYU doesn't modify the inclusion.
CommentPragmasD14 cpd14;

// d17.h is a private file with suggested include "no_such_file_d17.h".
// IWYU wants to include no_such_file_d17.h, even though it wants to
// keep d17.h, but is prohibited from doing so by a no_include pragma.
// IWYU: CommentPragmasD17 is...*no_such_file_d17.h
CommentPragmasD17 cpd17;

// Use cpd18,cpd19,cpd20a,cpd20b,cpd20c in ways that IWYU would normally want
// to suggest a forward declaration for. However, there's a
// no_forward_declare pragma in this file inhibiting the forward
// declarations.
CommentPragmasD18* cpd18;
CommentPragmasD19* cpd19;
CommentPragmasD20a* cpd20a;
Foo::CommentPragmasD20b* cpd20b;
CommentPragmasD20c* cpd20c;
// This is a case where IWYU wants the full definition of
// CommentPragmasTest21a due to the typedef, but then downgrades to
// requiring a forward declaration since the definition appears later
// in the same file. This forward declaration is inhibited due to a
// no_forward_declare pragma at the top of this file.
typedef CommentPragmasTest21a CommentPragmasTest21b;
class CommentPragmasTest21a {};

/**** IWYU_SUMMARY

tests/cxx/comment_pragmas.cc should add these lines:
#include <some_system_header_file>
#include "tests/cxx/comment_pragmas-i1.h"
#include "tests/cxx/comment_pragmas-i6.h"
#include "tests/cxx/comment_pragmas-i7.h"
#include "tests/cxx/comment_pragmas-i8.h"
#include "tests/cxx/indirect.h"
#include "tests/cxx/no_such_file.h"

tests/cxx/comment_pragmas.cc should remove these lines:
- #include "tests/cxx/comment_pragmas-d1.h"  // lines XX-XX
- #include "tests/cxx/comment_pragmas-d10.h"  // lines XX-XX
- #include "tests/cxx/comment_pragmas-d2.h"  // lines XX-XX
- #include "tests/cxx/comment_pragmas-d3.h"  // lines XX-XX
- #include "tests/cxx/comment_pragmas-d4.h"  // lines XX-XX
- #include "tests/cxx/comment_pragmas-d7.h"  // lines XX-XX
- #include "tests/cxx/comment_pragmas-d8.h"  // lines XX-XX
- #include "tests/cxx/comment_pragmas-d9.h"  // lines XX-XX
- class CommentPragmasD19;  // lines XX-XX
- class CommentPragmasTest21a;  // lines XX-XX

The full include-list for tests/cxx/comment_pragmas.cc:
#include <some_system_header_file>  // for CommentPragmasD8, CommentPragmasD9
#include "tests/cxx/comment_pragmas-d11.h"  // for CommentPragmasD11
#include "tests/cxx/comment_pragmas-d12.h"  // for CommentPragmasD12
#include "tests/cxx/comment_pragmas-d13.h"  // for CommentPragmasI10
#include "tests/cxx/comment_pragmas-d14.h"  // for CommentPragmasD14
#include "tests/cxx/comment_pragmas-d15.h"
#include "tests/cxx/comment_pragmas-d16.h"  // for CommentPragmasI11
#include "tests/cxx/comment_pragmas-d17.h"
#include "tests/cxx/comment_pragmas-d18.h"  // for CommentPragmasD18
#include "tests/cxx/comment_pragmas-d19.h"  // for CommentPragmasD19
#include "tests/cxx/comment_pragmas-d20.h"  // for CommentPragmasD20a, CommentPragmasD20b, CommentPragmasD20c
#include "tests/cxx/comment_pragmas-d21.h"
#include "tests/cxx/comment_pragmas-d21.h"
#include "tests/cxx/comment_pragmas-d22.h"
#include "tests/cxx/comment_pragmas-d22.h"
#include "tests/cxx/comment_pragmas-d5.h"
#include "tests/cxx/comment_pragmas-d6.h"
#include "tests/cxx/comment_pragmas-i1.h"  // for CommentPragmasI2, CommentPragmasI3, CommentPragmasI4
#include "tests/cxx/comment_pragmas-i6.h"  // for CommentPragmasD3
#include "tests/cxx/comment_pragmas-i7.h"  // for CommentPragmasD4
#include "tests/cxx/comment_pragmas-i8.h"  // for CommentPragmasI8
#include "tests/cxx/indirect.h"  // for IndirectClass
#include "tests/cxx/no_such_file.h"  // for CommentPragmasD2
class ForwardDeclaredUnnecessary1;  // lines XX-XX
class ForwardDeclaredUnnecessary2;  // lines XX-XX

***** IWYU_SUMMARY */
