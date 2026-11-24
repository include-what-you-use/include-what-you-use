//===--- specialization_use_window.cc - test input ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++17 \
//            -Xiwyu --mapping_file=tests/cxx/specialization_use_window.imp

// Tests that a use repeated for a revisited specialization type takes the
// header it would get at the place of the revisit. Holder uses Box<Payload>
// three times, the third one after the first use of Crate<Payload>. Box and
// Crate map to two public headers each, and which of them a use gets depends
// on the headers picked for the uses before it: the first two uses of Box get
// "-b.h", which is already needed for W, while the third one gets "-a.h",
// which the use of Crate made needed. So the third use must not be counted as
// another use of the second one, even though nothing tells them apart.

#include "tests/cxx/specialization_use_window-a.h"
#include "tests/cxx/specialization_use_window-b.h"
#include "tests/cxx/specialization_use_window-c.h"
#include "tests/cxx/specialization_use_window-payload.h"
#include "tests/cxx/specialization_use_window-root.h"

Holder<Payload> h;
W w;

/**** IWYU_SUMMARY

tests/cxx/specialization_use_window.cc should add these lines:

tests/cxx/specialization_use_window.cc should remove these lines:
- #include "tests/cxx/specialization_use_window-c.h"  // lines XX-XX

The full include-list for tests/cxx/specialization_use_window.cc:
#include "tests/cxx/specialization_use_window-a.h"  // for Box, Crate
#include "tests/cxx/specialization_use_window-b.h"  // for Box, W
#include "tests/cxx/specialization_use_window-payload.h"  // for Payload
#include "tests/cxx/specialization_use_window-root.h"  // for Holder

***** IWYU_SUMMARY */
