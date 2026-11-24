//===--- specialization_use_member_count.cc - test input ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++17 \
//            -Xiwyu --mapping_file=tests/cxx/specialization_use_member_count.imp

// Tests that what IWYU reports while traversing the members of a class
// specialization is not repeated when it skips a revisit of a specialization
// type whose first traversal led to that class. Holder reaches Wrap<Payload>
// as a qualifier, which needs its members, through every path through the
// nested Twice aliases, but traverses the members of Wrap<Payload> only once.
// Payload and W both map to "-i.h" and "-j.h", in opposite order of
// preference, so the header that the most uses prefer wins: counting the use
// of Payload as a member once per path would make "-i.h" win.

#include "tests/cxx/specialization_use_member_count-direct.h"
#include "tests/cxx/specialization_use_member_count-other-private.h"

// IWYU: Holder is...*specialization_use_member_count-root.h
// IWYU: Payload is...*specialization_use_member_count-j.h
// IWYU: Payload needs a declaration
Holder<Payload> h;
// IWYU: W is...*specialization_use_member_count-j.h
W w1, w2, w3, w4;

/**** IWYU_SUMMARY

tests/cxx/specialization_use_member_count.cc should add these lines:
#include "tests/cxx/specialization_use_member_count-j.h"
#include "tests/cxx/specialization_use_member_count-root.h"

tests/cxx/specialization_use_member_count.cc should remove these lines:
- #include "tests/cxx/specialization_use_member_count-direct.h"  // lines XX-XX
- #include "tests/cxx/specialization_use_member_count-other-private.h"  // lines XX-XX

The full include-list for tests/cxx/specialization_use_member_count.cc:
#include "tests/cxx/specialization_use_member_count-j.h"  // for Payload, W
#include "tests/cxx/specialization_use_member_count-root.h"  // for Holder

***** IWYU_SUMMARY */
