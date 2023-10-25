//===--- forward_declare_in_macro.cc - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// This test case used to be about proving that forward-declares inside macros
// are not reported for a line range beginning in the macro-spelling location
// (forward_declare_in_macro.h:11) and ending in the macro-expansion location
// (forward_declare_in_macro.cc:XX). So it was important that the macro
// expansion below was not on line 11, or the 'lines XX-XX' diagnostic would
// still match.
//
// We've since abandoned trying to remove whole macros when a forward
// declaration in them is unused, because a macro can do many useful things
// _and also_ forward-declare a symbol.
//
// We'll maintain the line 11 constraint and leave this note here, in case
// someone wants to try and recover the old behavior.

#include "tests/cxx/forward_declare_in_macro.h"

// Forward declaration unused but entirely inside a macro, will not be removed.
FORWARD_DECLARE_CLASS(MyKeepClass);

// The _declaration_ is in this file, but the symbol name is conjured using a
// macro. This is ostensibly safe to remove, but we keep it.
class CLASSNAME(RemoveClass);

// TAIL_DECL generates a class name (OneConflicted) from the argument, but
// injects another forward decl entirely inside the macro (TwoConflicted).
// Will be kept even though none of the decls are used.
class TAIL_DECL(Conflicted);

class NormalFwdDecl;

/**** IWYU_SUMMARY

tests/cxx/forward_declare_in_macro.cc should add these lines:

tests/cxx/forward_declare_in_macro.cc should remove these lines:
- class NormalFwdDecl;  // lines XX-XX

The full include-list for tests/cxx/forward_declare_in_macro.cc:
#include "tests/cxx/forward_declare_in_macro.h"
class MyKeepClass;  // lines XX-XX
class MyRemoveClass;  // lines XX-XX
class OneConflicted;  // lines XX-XX
class TwoConflicted;  // lines XX-XX

***** IWYU_SUMMARY */
