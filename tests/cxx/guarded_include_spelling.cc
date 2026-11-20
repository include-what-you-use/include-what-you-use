//===--- guarded_include_spelling.cc - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests the header that a file guarded by a macro of its includer is mapped
// to.  -i2.h can only be #included by -i1.h, which -d1.h #includes with a
// name relative to its own directory.  That name doesn't work from here:
// IWYU must suggest the name it gets from the search path.

#include "tests/cxx/subdir/guarded_include_spelling-d1.h"

SpellingD1 d1;
// IWYU: SpellingI1 is...*subdir/guarded_include_spelling-i1.h
SpellingI1 i1;
// IWYU: SpellingI2 is...*subdir/guarded_include_spelling-i1.h
SpellingI2 i2;

/**** IWYU_SUMMARY

tests/cxx/guarded_include_spelling.cc should add these lines:
#include "tests/cxx/subdir/guarded_include_spelling-i1.h"

tests/cxx/guarded_include_spelling.cc should remove these lines:

The full include-list for tests/cxx/guarded_include_spelling.cc:
#include "tests/cxx/subdir/guarded_include_spelling-d1.h"  // for SpellingD1
#include "tests/cxx/subdir/guarded_include_spelling-i1.h"  // for SpellingI1, SpellingI2

***** IWYU_SUMMARY */
