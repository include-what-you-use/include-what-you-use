//===--- provided_type_components_shared.cc - test input ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++11

#include "tests/cxx/provided_type_components_shared-aliases.h"

template <class T>
struct Holder {
  T value;
};

// Sharing descendants must not erase the provision carried by an alias.
Holder<ProvidedPayload> provided;

// A separate outer computation must not inherit the alias's provision.
// IWYU: Payload is...*provided_type_components_shared-payload.h
// IWYU: Payload needs a declaration
Holder<Duplicate<Duplicate<Duplicate<Payload>>>> ordinary;

/**** IWYU_SUMMARY

tests/cxx/provided_type_components_shared.cc should add these lines:
#include "tests/cxx/provided_type_components_shared-payload.h"

tests/cxx/provided_type_components_shared.cc should remove these lines:

The full include-list for tests/cxx/provided_type_components_shared.cc:
#include "tests/cxx/provided_type_components_shared-aliases.h"  // for Duplicate, ProvidedPayload
#include "tests/cxx/provided_type_components_shared-payload.h"  // for Payload

***** IWYU_SUMMARY */
