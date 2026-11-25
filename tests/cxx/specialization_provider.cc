//===--- specialization_provider.cc - test input --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++11

#include "tests/cxx/specialization_provider-direct.h"

// A providing first path must not suppress the later unprovided requirement.
// IWYU: Box is...*specialization_provider-inst.h
// IWYU: Root is...*specialization_provider-root.h
// IWYU: Payload is...*specialization_provider-payload.h
// IWYU: Provided is...*specialization_provider-provided.h
// IWYU: NonProvided is...*specialization_provider-nonprovided.h
// IWYU: Payload needs a declaration
Root<Provided, NonProvided, Payload> forward;

// Reversing the paths must preserve the same requirement.
// IWYU: Box is...*specialization_provider-inst.h
// IWYU: Root is...*specialization_provider-root.h
// IWYU: Payload is...*specialization_provider-payload.h
// IWYU: Provided is...*specialization_provider-provided.h
// IWYU: NonProvided is...*specialization_provider-nonprovided.h
// IWYU: Payload needs a declaration
Root<NonProvided, Provided, Payload> reverse;

// Function instantiation has its own traversal and enclosing-cache scope.
void Fn() {
  // IWYU: Provided is...*specialization_provider-provided.h
  // IWYU: NonProvided is...*specialization_provider-nonprovided.h
  // IWYU: Payload needs a declaration
  // IWYU: Box is...*specialization_provider-inst.h
  // IWYU: Payload is...*specialization_provider-payload.h
  // IWYU: UseInBody() is...*specialization_provider-root.h
  UseInBody<Provided, NonProvided, Payload>();
}

// Expansion and spelling files differ, but provision follows the alias header.
// IWYU: Box is...*specialization_provider-inst.h
// IWYU: RepeatedRoot is...*specialization_provider-root.h
// IWYU: Payload is...*specialization_provider-payload.h
// IWYU: ProvidedByMacro is...*specialization_provider-provided.h
// IWYU: NonProvidedByMacro is...*specialization_provider-nonprovided.h
// IWYU: Payload needs a declaration
RepeatedRoot<ProvidedByMacro, NonProvidedByMacro, Payload> macro;

// Any Box diagnostic here is unexpected, even though earlier callers need it.
// IWYU: Provided is...*specialization_provider-provided.h
// IWYU: ProvidedByMacro is...*specialization_provider-provided.h
// IWYU: Payload is...*specialization_provider-payload.h
// IWYU: RepeatedRoot is...*specialization_provider-root.h
// IWYU: Payload needs a declaration
RepeatedRoot<Provided, ProvidedByMacro, Payload> all_provided;

/**** IWYU_SUMMARY

tests/cxx/specialization_provider.cc should add these lines:
#include "tests/cxx/specialization_provider-inst.h"
#include "tests/cxx/specialization_provider-nonprovided.h"
#include "tests/cxx/specialization_provider-payload.h"
#include "tests/cxx/specialization_provider-provided.h"
#include "tests/cxx/specialization_provider-root.h"

tests/cxx/specialization_provider.cc should remove these lines:
- #include "tests/cxx/specialization_provider-direct.h"  // lines XX-XX

The full include-list for tests/cxx/specialization_provider.cc:
#include "tests/cxx/specialization_provider-inst.h"  // for Box
#include "tests/cxx/specialization_provider-nonprovided.h"  // for NonProvided, NonProvidedByMacro
#include "tests/cxx/specialization_provider-payload.h"  // for Payload
#include "tests/cxx/specialization_provider-provided.h"  // for Provided, ProvidedByMacro
#include "tests/cxx/specialization_provider-root.h"  // for RepeatedRoot, Root, UseInBody

***** IWYU_SUMMARY */
