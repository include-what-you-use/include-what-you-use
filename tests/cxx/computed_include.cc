//===--- computed_include.cc - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests computed #includes, i.e. #includes with macros.

#include "tests/cxx/computed_include.h"

// Test macro defined in another file.
#include MACRO_INC
// Test when #include with macro from another file is skipped.
#include MACRO_INC

// Macros with angle brackets differ from macros with quotation marks: the last
// include token is '>' and '"foo.h"' respectively.
#define MACRO_ANGLED_INC <stdio.h>
#include MACRO_ANGLED_INC
// And test how such #include is skipped.
#include MACRO_ANGLED_INC

// Test macro with arguments.
#define STRINGIZE(x) #x
#include STRINGIZE(tests/cxx/computed_include.h)

IndirectClass ic;

/**** IWYU_SUMMARY

tests/cxx/computed_include.cc should add these lines:

tests/cxx/computed_include.cc should remove these lines:
- #include "tests/cxx/computed_include.h"  // lines XX-XX
- #include <stdio.h>  // lines XX-XX
- #include <stdio.h>  // lines XX-XX
- #include "tests/cxx/indirect.h"  // lines XX-XX

The full include-list for tests/cxx/computed_include.cc:
#include "tests/cxx/computed_include.h"
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
