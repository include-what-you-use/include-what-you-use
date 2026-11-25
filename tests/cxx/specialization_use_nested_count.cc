//===--- specialization_use_nested_count.cc - test input ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++17 \
//            -Xiwyu --mapping_file=tests/cxx/specialization_use_nested_count.imp

// Nested alias paths must not outweigh independently written uses of W.
// Box and W have opposite mapped-header preferences, so preserving the
// path-multiplied counts would select the other header.

#include "tests/cxx/specialization_use_nested_count-direct.h"
#include "tests/cxx/specialization_use_nested_count-other-private.h"

// IWYU: Holder is...*specialization_use_nested_count-root.h
// IWYU: Payload is...*specialization_use_nested_count-payload.h
// IWYU: Box is...*specialization_use_nested_count-j.h...*for explicit instantiation
// IWYU: Payload needs a declaration
Holder<Payload> h;
// IWYU: W is...*specialization_use_nested_count-j.h
W w1, w2, w3, w4, w5, w6, w7, w8, w9, w10;

/**** IWYU_SUMMARY

tests/cxx/specialization_use_nested_count.cc should add these lines:
#include "tests/cxx/specialization_use_nested_count-j.h"
#include "tests/cxx/specialization_use_nested_count-payload.h"
#include "tests/cxx/specialization_use_nested_count-root.h"

tests/cxx/specialization_use_nested_count.cc should remove these lines:
- #include "tests/cxx/specialization_use_nested_count-direct.h"  // lines XX-XX
- #include "tests/cxx/specialization_use_nested_count-other-private.h"  // lines XX-XX

The full include-list for tests/cxx/specialization_use_nested_count.cc:
#include "tests/cxx/specialization_use_nested_count-j.h"  // for Box, W
#include "tests/cxx/specialization_use_nested_count-payload.h"  // for Payload
#include "tests/cxx/specialization_use_nested_count-root.h"  // for Holder

***** IWYU_SUMMARY */
