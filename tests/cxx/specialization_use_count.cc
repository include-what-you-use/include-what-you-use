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

// Extra alias paths must not give Box more influence than independently
// written uses of T, U and V when selecting among mapped headers.

#include "tests/cxx/specialization_use_count-direct.h"
#include "tests/cxx/specialization_use_count-other-private.h"

// IWYU: Holder is...*specialization_use_count-root.h
// IWYU: Payload is...*specialization_use_count-payload.h
// IWYU: Box is...*specialization_use_count-i.h...*for explicit instantiation
// IWYU: Payload needs a declaration
Holder<Payload> h;
// IWYU: T is...*specialization_use_count-h2.h
T t;
// IWYU: U is...*specialization_use_count-h2.h
U u;
// IWYU: V is...*specialization_use_count-h3.h
V v;

/**** IWYU_SUMMARY

tests/cxx/specialization_use_count.cc should add these lines:
#include "tests/cxx/specialization_use_count-h2.h"
#include "tests/cxx/specialization_use_count-h3.h"
#include "tests/cxx/specialization_use_count-i.h"
#include "tests/cxx/specialization_use_count-payload.h"
#include "tests/cxx/specialization_use_count-root.h"

tests/cxx/specialization_use_count.cc should remove these lines:
- #include "tests/cxx/specialization_use_count-direct.h"  // lines XX-XX
- #include "tests/cxx/specialization_use_count-other-private.h"  // lines XX-XX

The full include-list for tests/cxx/specialization_use_count.cc:
#include "tests/cxx/specialization_use_count-h2.h"  // for T, U
#include "tests/cxx/specialization_use_count-h3.h"  // for V
#include "tests/cxx/specialization_use_count-i.h"  // for Box
#include "tests/cxx/specialization_use_count-payload.h"  // for Payload
#include "tests/cxx/specialization_use_count-root.h"  // for Holder

***** IWYU_SUMMARY */
