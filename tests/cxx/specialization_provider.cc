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

// Provided's header supplies the explicit instantiation, but NonProvided's
// does not. The shared Box node must report a use on the second full-use path.
// IWYU: Box is...*specialization_provider-inst.h
// IWYU: Root is...*specialization_provider-root.h
// IWYU: Payload is...*specialization_provider-payload.h
// IWYU: Provided is...*specialization_provider-provided.h
// IWYU: NonProvided is...*specialization_provider-nonprovided.h
// IWYU: Payload needs a declaration
Root<Provided, NonProvided, Payload> instance;

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
#include "tests/cxx/specialization_provider-nonprovided.h"  // for NonProvided
#include "tests/cxx/specialization_provider-payload.h"  // for Payload
#include "tests/cxx/specialization_provider-provided.h"  // for Provided
#include "tests/cxx/specialization_provider-root.h"  // for Root

***** IWYU_SUMMARY */
