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

// Redundant paths after Crate must not introduce a second Box attribution.
// Both required headers remain necessary for distinct uses of Crate and W.

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
#include "tests/cxx/specialization_use_window-a.h"  // for Crate
#include "tests/cxx/specialization_use_window-b.h"  // for Box, W
#include "tests/cxx/specialization_use_window-payload.h"  // for Payload
#include "tests/cxx/specialization_use_window-root.h"  // for Holder

***** IWYU_SUMMARY */
