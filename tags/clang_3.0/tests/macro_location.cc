//===--- macro_location.cc - test input file for iwyu ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests a few common types of macro uses to make sure we correctly
// identify what symbols belong to the macro author, and what symbols
// belong to the macro user.  Also make sure we don't ignore macro
// expansions when the macro is written in a to-ignore file.

#include "tests/macro_location.h"

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

/**** IWYU_SUMMARY

(tests/macro_location.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
