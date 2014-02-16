//===--- redecls.cc - test input file for iwyu ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// For types that can be declared in many places -- functions,
// typedefs, variables, and the like -- make sure that we accept
// any declaration as satisfying a use.  In order to minimize
// chances that we're only looking at one of the redecls but are
// happening to pick the one in the same file as the use, we
// put the use-file in the "middle".


#include "tests/cxx/redecls-d1.h"
#include "tests/cxx/redecls.h"
#include "tests/cxx/redecls-d2.h"

/**** IWYU_SUMMARY

tests/cxx/redecls.cc should add these lines:

tests/cxx/redecls.cc should remove these lines:
- #include "tests/cxx/redecls-d1.h"  // lines XX-XX
- #include "tests/cxx/redecls-d2.h"  // lines XX-XX

The full include-list for tests/cxx/redecls.cc:
#include "tests/cxx/redecls.h"

***** IWYU_SUMMARY */
