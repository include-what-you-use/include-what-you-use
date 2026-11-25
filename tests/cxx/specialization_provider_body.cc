//===--- specialization_provider_body.cc - test input ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++11

#include "tests/cxx/specialization_provider-direct.h"

// The type is traversed in an instantiated function body.
void Fn() {
  // IWYU: Provided is...*specialization_provider-provided.h
  // IWYU: NonProvided is...*specialization_provider-nonprovided.h
  // IWYU: Payload needs a declaration
  // IWYU: Box is...*specialization_provider-inst.h
  // IWYU: Payload is...*specialization_provider-payload.h
  // IWYU: UseInBody() is...*specialization_provider-root.h
  UseInBody<Provided, NonProvided, Payload>();
}

/**** IWYU_SUMMARY

tests/cxx/specialization_provider_body.cc should add these lines:
#include "tests/cxx/specialization_provider-inst.h"
#include "tests/cxx/specialization_provider-nonprovided.h"
#include "tests/cxx/specialization_provider-payload.h"
#include "tests/cxx/specialization_provider-provided.h"
#include "tests/cxx/specialization_provider-root.h"

tests/cxx/specialization_provider_body.cc should remove these lines:
- #include "tests/cxx/specialization_provider-direct.h"  // lines XX-XX

The full include-list for tests/cxx/specialization_provider_body.cc:
#include "tests/cxx/specialization_provider-inst.h"  // for Box
#include "tests/cxx/specialization_provider-nonprovided.h"  // for NonProvided
#include "tests/cxx/specialization_provider-payload.h"  // for Payload
#include "tests/cxx/specialization_provider-provided.h"  // for Provided
#include "tests/cxx/specialization_provider-root.h"  // for UseInBody

***** IWYU_SUMMARY */
