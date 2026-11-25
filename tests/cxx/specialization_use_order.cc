//===--- specialization_use_order.cc - test input -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++17 \
//            -Xiwyu --mapping_file=tests/cxx/specialization_use_order.imp

// A redundant path after Crate must not add another attribution of Box.
// The retained distinct uses still require both headers independently.

#include "tests/cxx/specialization_use_order-a.h"
#include "tests/cxx/specialization_use_order-b.h"
#include "tests/cxx/specialization_use_order-c.h"
#include "tests/cxx/specialization_use_order-payload.h"
#include "tests/cxx/specialization_use_order-root.h"

Holder<Payload> h;
W w;

/**** IWYU_SUMMARY

tests/cxx/specialization_use_order.cc should add these lines:

tests/cxx/specialization_use_order.cc should remove these lines:
- #include "tests/cxx/specialization_use_order-c.h"  // lines XX-XX

The full include-list for tests/cxx/specialization_use_order.cc:
#include "tests/cxx/specialization_use_order-a.h"  // for Crate
#include "tests/cxx/specialization_use_order-b.h"  // for Box, W
#include "tests/cxx/specialization_use_order-payload.h"  // for Payload
#include "tests/cxx/specialization_use_order-root.h"  // for Holder

***** IWYU_SUMMARY */
