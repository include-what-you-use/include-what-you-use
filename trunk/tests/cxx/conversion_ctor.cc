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

#include "tests/cxx/direct.h"

// IWYU: IndirectClass is...*indirect.h
IndirectClass ic;  // this triggers generation of implicit move constructor

// IWYU: IndirectClass needs a declaration
char Foo(IndirectClass); // this checks if IndirectClass has an implicit
                         // conversion constructor, which misfires because
                         // there's an implicit move constructor.

// The patch to issue #89 fixes this by having iwyu_ast_util.cc's
// HasImplicitConversionCtor not treat implicit move constructors as
// implicit conversion constructors.

/**** IWYU_SUMMARY

tests/cxx/conversion_ctor.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/conversion_ctor.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/conversion_ctor.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
