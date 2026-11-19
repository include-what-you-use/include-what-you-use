//===--- type_enumerator_sugar.cc - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests that every spelling of a type in a template argument is considered
// for providing it, even when the type behind the spelling has been seen in
// the argument already. Both typedefs below name the same type node, but only
// the one from a header provides Payload.

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
