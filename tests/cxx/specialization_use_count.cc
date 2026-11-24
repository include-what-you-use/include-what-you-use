//===--- specialization_use_count.cc - test input -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++17 \
//            -Xiwyu --mapping_file=tests/cxx/specialization_use_count.imp

// Tests that a specialization type that IWYU reaches through many paths
// counts as used once per path, even though IWYU doesn't traverse it again
// for each of them. Box, T, U and V each map to two public headers, and IWYU
// prefers the headers that the most uses can come from. Counting each
// explicit instantiation use of Box<Payload> only once would make "-h2.h"
// (for T and U) look as useful as "-i.h" (for Box and T).

#include "tests/cxx/specialization_use_count-direct.h"
#include "tests/cxx/specialization_use_count-other-private.h"

// IWYU: Holder is...*specialization_use_count-root.h
// IWYU: Payload is...*specialization_use_count-payload.h
// IWYU: Box is...*specialization_use_count-i.h...*for explicit instantiation
// IWYU: Payload needs a declaration
Holder<Payload> h;
// IWYU: T is...*specialization_use_count-i.h
T t;
// IWYU: U is...*specialization_use_count-h3.h
U u;
// IWYU: V is...*specialization_use_count-h3.h
V v;

/**** IWYU_SUMMARY

tests/cxx/specialization_use_count.cc should add these lines:
#include "tests/cxx/specialization_use_count-h3.h"
#include "tests/cxx/specialization_use_count-i.h"
#include "tests/cxx/specialization_use_count-payload.h"
#include "tests/cxx/specialization_use_count-root.h"

tests/cxx/specialization_use_count.cc should remove these lines:
- #include "tests/cxx/specialization_use_count-direct.h"  // lines XX-XX
- #include "tests/cxx/specialization_use_count-other-private.h"  // lines XX-XX

The full include-list for tests/cxx/specialization_use_count.cc:
#include "tests/cxx/specialization_use_count-h3.h"  // for U, V
#include "tests/cxx/specialization_use_count-i.h"  // for Box, T
#include "tests/cxx/specialization_use_count-payload.h"  // for Payload
#include "tests/cxx/specialization_use_count-root.h"  // for Holder

***** IWYU_SUMMARY */
