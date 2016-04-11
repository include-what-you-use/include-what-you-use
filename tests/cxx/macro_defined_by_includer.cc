//===--- macro_defined_by_includer.cc - test input file for iwyu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests a few macro patterns:
// * internal headers guarded by macro defined in designated header;
// * x-macros.
//
// Usually you include a file with a macro and use that macro.  But in these
// macro patterns macro is defined by includer and includee doesn't know where
// the macro comes from.


// Test guarded includes.
#include "tests/cxx/macro_defined_by_includer-d1.h"
// IWYU: GuardedInclude1 is...*macro_defined_by_includer-i1.h
GuardedInclude1 g1;

#define DIRECT_INCLUDE_GUARD_2
#include "tests/cxx/macro_defined_by_includer-g2.h"
GuardedInclude2 g2;


// Test x-macros.
#include "tests/cxx/macro_defined_by_includer-d2.h"

#define TYPE int
#include "tests/cxx/macro_defined_by_includer-xmacro.h"
#undef TYPE

// For x-macros we keep all includes even if its content isn't used.
#define TYPE double
#include "tests/cxx/macro_defined_by_includer-xmacro.h"
#undef TYPE

int main() {
  // IWYU: f is...*macro_defined_by_includer-i2.h
  f(3);
  // IWYU: f is...*macro_defined_by_includer-i2.h
  f('a');
}


// Test macro defined on command line.  Make sure that detecting file defining
// macro works without actual file on disk.
COMMAND_LINE_TYPE x;


/**** IWYU_SUMMARY

tests/cxx/macro_defined_by_includer.cc should add these lines:
#include "tests/cxx/macro_defined_by_includer-i1.h"
#include "tests/cxx/macro_defined_by_includer-i2.h"

tests/cxx/macro_defined_by_includer.cc should remove these lines:
- #include "tests/cxx/macro_defined_by_includer-d1.h"  // lines XX-XX
- #include "tests/cxx/macro_defined_by_includer-d2.h"  // lines XX-XX

The full include-list for tests/cxx/macro_defined_by_includer.cc:
#include "tests/cxx/macro_defined_by_includer-g2.h"  // for GuardedInclude2
#include "tests/cxx/macro_defined_by_includer-i1.h"  // for GuardedInclude1
#include "tests/cxx/macro_defined_by_includer-i2.h"  // for f
#include "tests/cxx/macro_defined_by_includer-xmacro.h"  // lines XX-XX
#include "tests/cxx/macro_defined_by_includer-xmacro.h"  // lines XX-XX

***** IWYU_SUMMARY */
