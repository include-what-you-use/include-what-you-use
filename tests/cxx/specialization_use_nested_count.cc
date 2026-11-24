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

// Tests that the uses repeated for a specialization type revisited inside
// another revisited one are counted as often as the paths to them. Box and W
// both map to "-i.h" and "-j.h", in opposite order of preference, so the
// header that the most uses prefer wins. Holder reaches Q<Box<Payload>>, which
// reports the explicit instantiation of Box<Payload>, through every path
// through the nested Twice aliases, which outnumber the uses of W only if the
// repeats of the inner Twice are counted for each repeat of the outer ones.

#include "tests/cxx/specialization_use_nested_count-direct.h"
#include "tests/cxx/specialization_use_nested_count-other-private.h"

// IWYU: Holder is...*specialization_use_nested_count-root.h
// IWYU: Payload is...*specialization_use_nested_count-payload.h
// IWYU: Box is...*specialization_use_nested_count-i.h...*for explicit instantiation
// IWYU: Payload needs a declaration
Holder<Payload> h;
// IWYU: W is...*specialization_use_nested_count-i.h
W w1, w2, w3, w4, w5, w6, w7, w8, w9, w10;

/**** IWYU_SUMMARY

tests/cxx/specialization_use_nested_count.cc should add these lines:
#include "tests/cxx/specialization_use_nested_count-i.h"
#include "tests/cxx/specialization_use_nested_count-payload.h"
#include "tests/cxx/specialization_use_nested_count-root.h"

tests/cxx/specialization_use_nested_count.cc should remove these lines:
- #include "tests/cxx/specialization_use_nested_count-direct.h"  // lines XX-XX
- #include "tests/cxx/specialization_use_nested_count-other-private.h"  // lines XX-XX

The full include-list for tests/cxx/specialization_use_nested_count.cc:
#include "tests/cxx/specialization_use_nested_count-i.h"  // for Box, W
#include "tests/cxx/specialization_use_nested_count-payload.h"  // for Payload
#include "tests/cxx/specialization_use_nested_count-root.h"  // for Holder

***** IWYU_SUMMARY */
