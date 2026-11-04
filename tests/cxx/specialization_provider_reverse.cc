//===--- specialization_provider_reverse.cc - test input ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++11

#include "tests/cxx/specialization_provider-direct.h"

// Reversing the paths must not change responsibility for the explicit
// instantiation. This also catches a forward visit suppressing a full visit.
// IWYU: Box is...*specialization_provider-inst.h
// IWYU: Root is...*specialization_provider-root.h
// IWYU: Payload is...*specialization_provider-payload.h
// IWYU: NonProvided is...*specialization_provider-nonprovided.h
// IWYU: Provided is...*specialization_provider-provided.h
// IWYU: Payload needs a declaration
Root<NonProvided, Provided, Payload> instance;

/**** IWYU_SUMMARY

tests/cxx/specialization_provider_reverse.cc should add these lines:
#include "tests/cxx/specialization_provider-inst.h"
#include "tests/cxx/specialization_provider-nonprovided.h"
#include "tests/cxx/specialization_provider-payload.h"
#include "tests/cxx/specialization_provider-provided.h"
#include "tests/cxx/specialization_provider-root.h"

tests/cxx/specialization_provider_reverse.cc should remove these lines:
- #include "tests/cxx/specialization_provider-direct.h"  // lines XX-XX

The full include-list for tests/cxx/specialization_provider_reverse.cc:
#include "tests/cxx/specialization_provider-inst.h"  // for Box
#include "tests/cxx/specialization_provider-nonprovided.h"  // for NonProvided
#include "tests/cxx/specialization_provider-payload.h"  // for Payload
#include "tests/cxx/specialization_provider-provided.h"  // for Provided
#include "tests/cxx/specialization_provider-root.h"  // for Root

***** IWYU_SUMMARY */
