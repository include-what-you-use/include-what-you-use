//===--- specialization_use_count_heavier.cc - test input -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++17 \
//            -Xiwyu --mapping_file=tests/cxx/specialization_use_count.imp

// Independent written uses retain their influence even when an alias graph
// exposes exponentially many paths to Box<Payload>.

#include "tests/cxx/specialization_use_count-direct.h"
#include "tests/cxx/specialization_use_count-other-private.h"

// IWYU: Holder is...*specialization_use_count-root.h
// IWYU: Payload is...*specialization_use_count-payload.h
// IWYU: Box is...*specialization_use_count-i.h...*for explicit instantiation
// IWYU: Payload needs a declaration
Holder<Payload> h;
// IWYU: T is...*specialization_use_count-h2.h
T t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
// IWYU: U is...*specialization_use_count-h2.h
U u1, u2, u3, u4, u5, u6, u7, u8, u9, u10;
// IWYU: V is...*specialization_use_count-h3.h
V v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;

/**** IWYU_SUMMARY

tests/cxx/specialization_use_count_heavier.cc should add these lines:
#include "tests/cxx/specialization_use_count-h2.h"
#include "tests/cxx/specialization_use_count-h3.h"
#include "tests/cxx/specialization_use_count-i.h"
#include "tests/cxx/specialization_use_count-payload.h"
#include "tests/cxx/specialization_use_count-root.h"

tests/cxx/specialization_use_count_heavier.cc should remove these lines:
- #include "tests/cxx/specialization_use_count-direct.h"  // lines XX-XX
- #include "tests/cxx/specialization_use_count-other-private.h"  // lines XX-XX

The full include-list for tests/cxx/specialization_use_count_heavier.cc:
#include "tests/cxx/specialization_use_count-h2.h"  // for T, U
#include "tests/cxx/specialization_use_count-h3.h"  // for V
#include "tests/cxx/specialization_use_count-i.h"  // for Box
#include "tests/cxx/specialization_use_count-payload.h"  // for Payload
#include "tests/cxx/specialization_use_count-root.h"  // for Holder

***** IWYU_SUMMARY */
