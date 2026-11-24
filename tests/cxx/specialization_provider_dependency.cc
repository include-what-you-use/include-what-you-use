//===--- specialization_provider_dependency.cc - test input ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++17 \
//            -Xiwyu --mapping_file=tests/cxx/specialization_provider_dependency.imp

// Tests that a specialization type that IWYU reaches through many paths
// counts as used along exactly the paths on which no enclosing template
// provides it.  Whether a path through Holder's nested aliases goes through
// the alias in "-provided.h" decides whether Box<Payload> is reported there,
// also for specialization types that only contain Box<Payload> further down.
// Box and T each map to "-i.h" and "-j.h" in opposite order, and IWYU picks
// the header listed first by the most uses.  T is used 30 times, so counting
// Box (25 uses) on a path where "-provided.h" provides it would pick "-i.h".

#include "tests/cxx/specialization_use_count-direct.h"
#include "tests/cxx/specialization_use_count-other-private.h"

// IWYU: Holder is...*specialization_use_count-root.h
// IWYU: Payload is...*specialization_use_count-payload.h
// IWYU: Box is...*specialization_use_count-j.h...*for explicit instantiation
// IWYU: Payload needs a declaration
Holder<Payload> h;

struct Uses {
  // IWYU: T is...*specialization_use_count-j.h
  T a0; T a1; T a2; T a3; T a4; T a5; T a6; T a7; T a8; T a9;
  // IWYU: T is...*specialization_use_count-j.h
  T b0; T b1; T b2; T b3; T b4; T b5; T b6; T b7; T b8; T b9;
  // IWYU: T is...*specialization_use_count-j.h
  T c0; T c1; T c2; T c3; T c4; T c5; T c6; T c7; T c8; T c9;
};

/**** IWYU_SUMMARY

tests/cxx/specialization_provider_dependency.cc should add these lines:
#include "tests/cxx/specialization_use_count-j.h"
#include "tests/cxx/specialization_use_count-payload.h"
#include "tests/cxx/specialization_use_count-root.h"

tests/cxx/specialization_provider_dependency.cc should remove these lines:
- #include "tests/cxx/specialization_use_count-direct.h"  // lines XX-XX
- #include "tests/cxx/specialization_use_count-other-private.h"  // lines XX-XX

The full include-list for tests/cxx/specialization_provider_dependency.cc:
#include "tests/cxx/specialization_use_count-j.h"  // for Box, T
#include "tests/cxx/specialization_use_count-payload.h"  // for Payload
#include "tests/cxx/specialization_use_count-root.h"  // for Holder

***** IWYU_SUMMARY */
