//===--- specialization_use_comment_order.cc - test input -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++17

#include "tests/cxx/specialization_use_alternating-aliases.h"
#include "tests/cxx/specialization_use_alternating-inst.h"

// Skipping redundant alias paths must not drop distinct symbols from the
// include comment. Summary matching ignores symbol order, so this checks
// presence rather than frequency-based ordering.
Alpha ordinary;
// IWYU: Payload needs a declaration
Holder<Payload> object;

/**** IWYU_SUMMARY

tests/cxx/specialization_use_comment_order.cc should add these lines:
struct Payload;

tests/cxx/specialization_use_comment_order.cc should remove these lines:

The full include-list for tests/cxx/specialization_use_comment_order.cc:
#include "tests/cxx/specialization_use_alternating-aliases.h"  // for Holder
#include "tests/cxx/specialization_use_alternating-inst.h"  // for Alpha, Box1, Box2
struct Payload;

***** IWYU_SUMMARY */
