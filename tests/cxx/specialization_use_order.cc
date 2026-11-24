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

// Tests that the uses of a specialization type that IWYU reaches again keep
// their place among the other uses, even though IWYU doesn't traverse it
// again. Box and Crate map to two public headers each, and which of them
// a use gets depends on the headers picked for the uses before it: the
// first use of Box gets "-b.h", which is already needed for W, while the one
// after the use of Crate gets "-a.h", which that use made needed.

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
#include "tests/cxx/specialization_use_order-a.h"  // for Box, Crate
#include "tests/cxx/specialization_use_order-b.h"  // for Box, W
#include "tests/cxx/specialization_use_order-payload.h"  // for Payload
#include "tests/cxx/specialization_use_order-root.h"  // for Holder

***** IWYU_SUMMARY */
