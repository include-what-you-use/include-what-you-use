//===--- specialization_use_alternating.cc - test input -------------------===//
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

// IWYU: Payload needs a declaration
Holder<Payload> object;

/**** IWYU_SUMMARY

tests/cxx/specialization_use_alternating.cc should add these lines:
struct Payload;

tests/cxx/specialization_use_alternating.cc should remove these lines:

The full include-list for tests/cxx/specialization_use_alternating.cc:
#include "tests/cxx/specialization_use_alternating-aliases.h"  // for Holder
#include "tests/cxx/specialization_use_alternating-inst.h"  // for Box1, Box2
struct Payload;

***** IWYU_SUMMARY */
