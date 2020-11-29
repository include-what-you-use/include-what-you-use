//===--- pragma_associated.cc - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

#include "tests/cxx/pragma_associated-d1.h"  // IWYU pragma: associated
#include "tests/cxx/pragma_associated-d2.h"  // IWYU pragma: associated
#include "tests/cxx/pragma_associated.h"  // This still counts as associated.
#include "tests/cxx/direct.h"  // This is unused.

/**** IWYU_SUMMARY

tests/cxx/pragma_associated.cc should add these lines:

tests/cxx/pragma_associated.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/pragma_associated.cc:
#include "tests/cxx/pragma_associated-d1.h"
#include "tests/cxx/pragma_associated-d2.h"
#include "tests/cxx/pragma_associated.h"

***** IWYU_SUMMARY */
