//===--- multiple_include_paths.cc - test input file for iwyu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that when a file is referred to in a non-canonical way, iwyu
// respects that rather than trying to rewrite it.  This matters most
// when a file can be referred to in two ways because of
// include-paths: if we #include "a/b/c.h" and compile with "-I. -Ia
// -Ia/b", then we could say #include "a/b/c.h", #include "b/c.h", or
// #include "c.h").  clang will, as I understand it, pick one of these
// three forms arbitrarily for FileEntry::getName.  We want to use the
// name that was actually written in the source.  It also matters when
// there are symlinks in the source-tree.
//
// However, those are both pretty tricky to test, so instead we
// just throw in some ..'s that clang would otherwise normalize.

// For a given file, clang stores the first includename-as-written as
// its canonical name.  The include of direct.h will make sure it's
// the one without the ..

#include "tests/direct.h"
#include "tests/internal/../indirect.h"

IndirectClass ic;


/**** IWYU_SUMMARY


tests/multiple_include_paths.cc should add these lines:

tests/multiple_include_paths.cc should remove these lines:
- #include "tests/direct.h"  // lines XX-XX

The full include-list for tests/multiple_include_paths.cc:
#include "tests/internal/../indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
