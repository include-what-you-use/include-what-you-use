//===--- specialization_use_comment_truncation.cc - test input ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++17 -Xiwyu --verbose=1

// Redundant alias paths must not push a distinct written use out of a
// truncated include comment. With equal counts Alpha sorts before Box1.

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
#include "tests/cxx/specialization_use_alternating-inst.h"  // for Alpha
struct Payload;

***** IWYU_SUMMARY */
