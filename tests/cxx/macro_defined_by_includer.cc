//===--- macro_defined_by_includer.cc - test input file for iwyu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -std=c++11 -DCOMMAND_LINE_TYPE=double \
//            -include tests/cxx/macro_defined_by_includer-prefix.h -I .

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

#include "tests/cxx/macro_defined_by_includer-d2.h"
GuardedInclude2 g2;

#include "tests/cxx/macro_defined_by_includer-d3.h"
GuardedInclude3 g3;
GuardedInclude4 g4;

#define DIRECT_INCLUDE_GUARD_5
#include "tests/cxx/macro_defined_by_includer-g5.h"
GuardedInclude5 g5;


// Test x-macros.
#include "tests/cxx/macro_defined_by_includer-d4.h"

#define TYPE int
#include "tests/cxx/macro_defined_by_includer-xmacro.h"
#undef TYPE

// For x-macros we keep all includes even if its content isn't used.
#define TYPE double
#include "tests/cxx/macro_defined_by_includer-xmacro.h"
#undef TYPE

int main() {
  // IWYU: f is...*macro_defined_by_includer-i3.h
  f(3);
  // IWYU: f is...*macro_defined_by_includer-i3.h
  f('a');
}


// Test macro defined on command line.  Make sure that detecting file defining
// macro works without actual file on disk.
COMMAND_LINE_TYPE x;


// Clang internal <limits.h> defines LLONG_MIN and #include_next system
// <limits.h> which on Mac OS X uses LLONG_MIN.
//
// Test that we don't create a mapping between those 2 <limits.h> and don't try
// to mark system <limits.h> as private.
#include <limits.h>


/**** IWYU_SUMMARY

tests/cxx/macro_defined_by_includer.cc should add these lines:
#include "tests/cxx/macro_defined_by_includer-i1.h"
#include "tests/cxx/macro_defined_by_includer-i3.h"

tests/cxx/macro_defined_by_includer.cc should remove these lines:
- #include <limits.h>  // lines XX-XX
- #include "tests/cxx/macro_defined_by_includer-d1.h"  // lines XX-XX
- #include "tests/cxx/macro_defined_by_includer-d4.h"  // lines XX-XX

The full include-list for tests/cxx/macro_defined_by_includer.cc:
#include "tests/cxx/macro_defined_by_includer-d2.h"  // for GuardedInclude2
#include "tests/cxx/macro_defined_by_includer-d3.h"  // for GuardedInclude3, GuardedInclude4
#include "tests/cxx/macro_defined_by_includer-g5.h"  // for GuardedInclude5
#include "tests/cxx/macro_defined_by_includer-i1.h"  // for GuardedInclude1
#include "tests/cxx/macro_defined_by_includer-i3.h"  // for f
#include "tests/cxx/macro_defined_by_includer-xmacro.h"  // lines XX-XX
#include "tests/cxx/macro_defined_by_includer-xmacro.h"  // lines XX-XX

***** IWYU_SUMMARY */
