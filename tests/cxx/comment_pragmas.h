//===--- comment_pragmas.h - test input file for iwyu ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU pragma: no_forward_declare CommentPragmasTest21aHeader

class CommentPragmasTest21aHeader;  // Needed but removed due to
                                    // no_forward_declare.

// This is a case where IWYU wants the full definition of
// CommentPragmasTest21aHeader due to the typedef, but then downgrades to
// requiring a forward declaration since the definition appears later
// in the same file. This forward declaration is inhibited due to a
// no_forward_declare pragma at the top of this file.
typedef CommentPragmasTest21aHeader CommentPragmasTest21bHeader;
class CommentPragmasTest21aHeader {};

/**** IWYU_SUMMARY

tests/cxx/comment_pragmas.h should add these lines:

tests/cxx/comment_pragmas.h should remove these lines:
- class CommentPragmasTest21aHeader;  // lines XX-XX

The full include-list for tests/cxx/comment_pragmas.h:

***** IWYU_SUMMARY */
