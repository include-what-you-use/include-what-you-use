//===--- macro_location.cc - test input file for iwyu ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --check_also="tests/cxx/*-d2.h" \
//            -Xiwyu --check_also="tests/cxx/*-d5.h" -Wno-sizeof-pointer-div \
//            -I .

// Tests a few common types of macro uses to make sure we correctly
// identify what symbols belong to the macro author, and what symbols
// belong to the macro user.  Also make sure we don't ignore macro
// expansions when the macro is written in a to-ignore file.

#include "tests/cxx/macro_location.h"
#include "tests/cxx/macro_location-d5.h"
#include "tests/cxx/macro_location-inet.h"

struct A {
  // This doesn't require us to forward-declare ToBeDeclared because it's
  // part of a friendship declaration.
  friend class ToBeDeclaredLater1;
  // iwyu should treat this exactly the same, even though DECLARE_FRIEND
  // is in a far-away file.
  // IWYU: DECLARE_FRIEND is...*macro_location-i3.h
  DECLARE_FRIEND(ToBeDeclaredLater2);
};

class ToBeDeclaredLater1 { };
class ToBeDeclaredLater2 { };

// This is lifted from a reduced repro case for htons of inet.h.
// Its nesting provokes something that isn't covered by the rest of the
// macro_location suite.
int my_htons(int x) {
  return iwyu_htons(x);
}

/**** IWYU_SUMMARY

tests/cxx/macro_location.cc should add these lines:

tests/cxx/macro_location.cc should remove these lines:
- #include "tests/cxx/macro_location-d5.h"  // lines XX-XX

The full include-list for tests/cxx/macro_location.cc:
#include "tests/cxx/macro_location.h"
#include "tests/cxx/macro_location-inet.h"  // for iwyu_htons

***** IWYU_SUMMARY */
