//===--- specialization_use_comment_truncation.cc - test input ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++17 -Xiwyu --verbose=1

// Tests that repeated specialization reports decide the order of the symbols
// in an include's comment. Summary matching sorts comment names, but below
// verbose level 3 IWYU truncates comments to the maximum line length, which
// here leaves room for the most frequent symbol only: Box1, which the
// alternating specialization types report many times, rather than Alpha,
// which sorts first alphabetically.

#include "tests/cxx/specialization_use_alternating-aliases.h"
#include "tests/cxx/specialization_use_alternating-inst.h"

Alpha ordinary;
Holder<Payload> object;

/**** IWYU_SUMMARY

tests/cxx/specialization_use_comment_truncation.cc should add these lines:
struct Payload;

tests/cxx/specialization_use_comment_truncation.cc should remove these lines:

The full include-list for tests/cxx/specialization_use_comment_truncation.cc:
#include "tests/cxx/specialization_use_alternating-aliases.h"  // for Holder
#include "tests/cxx/specialization_use_alternating-inst.h"  // for Box1
struct Payload;

***** IWYU_SUMMARY */
