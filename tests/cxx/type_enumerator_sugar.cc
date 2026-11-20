//===--- type_enumerator_sugar.cc - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Checks that collecting a new spelling precedes pruning repeated descent.
// Both typedefs name Payload, but only the header-defined typedef provides it.
// Instantiating Holder needs Payload complete; losing ProvidedPayload's
// providing information would incorrectly require the payload header here.
// This is a correctness test, not a performance test.

#include "tests/cxx/type_enumerator_sugar-provider.h"
#include "tests/cxx/type_enumerator_sugar-tpl.h"

// IWYU: Payload needs a declaration
typedef Payload LocalPayload;

Holder<Pair<LocalPayload, ProvidedPayload>> holder;

/**** IWYU_SUMMARY

tests/cxx/type_enumerator_sugar.cc should add these lines:
struct Payload;

tests/cxx/type_enumerator_sugar.cc should remove these lines:

The full include-list for tests/cxx/type_enumerator_sugar.cc:
#include "tests/cxx/type_enumerator_sugar-provider.h"  // for ProvidedPayload
#include "tests/cxx/type_enumerator_sugar-tpl.h"  // for Holder, Pair
struct Payload;

***** IWYU_SUMMARY */
