//===--- conversion_ctor.cc - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// The smallest repro case for issue #89:
// http://code.google.com/p/include-what-you-use/issues/detail?id=89

#include "tests/direct.h"

// IWYU: IndirectClass needs a declaration
char Foo(IndirectClass);

// IWYU: IndirectClass is...*indirect.h
IndirectClass ic;

/**** IWYU_SUMMARY

tests/conversion_ctor.cc should add these lines:
#include "tests/indirect.h"

tests/conversion_ctor.cc should remove these lines:
- #include "tests/direct.h"  // lines XX-XX

The full include-list for tests/conversion_ctor.cc:
#include "tests/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
