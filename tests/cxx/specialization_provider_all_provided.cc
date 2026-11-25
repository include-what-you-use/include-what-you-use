//===--- specialization_provider_all_provided.cc - test input -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++11

#include "tests/cxx/specialization_provider-direct.h"

// If every repeated path provides the explicit instantiation, reuse must not
// make the caller include its declaration header directly.
// IWYU: Provided is...*specialization_provider-provided.h
// IWYU: ProvidedByMacro is...*specialization_provider-provided.h
// IWYU: Payload is...*specialization_provider-payload.h
// IWYU: RepeatedRoot is...*specialization_provider-root.h
// IWYU: Payload needs a declaration
RepeatedRoot<Provided, ProvidedByMacro, Payload> instance;

/**** IWYU_SUMMARY

tests/cxx/specialization_provider_all_provided.cc should add these lines:
#include "tests/cxx/specialization_provider-payload.h"
#include "tests/cxx/specialization_provider-provided.h"
#include "tests/cxx/specialization_provider-root.h"

tests/cxx/specialization_provider_all_provided.cc should remove these lines:
- #include "tests/cxx/specialization_provider-direct.h"  // lines XX-XX

The full include-list for tests/cxx/specialization_provider_all_provided.cc:
#include "tests/cxx/specialization_provider-payload.h"  // for Payload
#include "tests/cxx/specialization_provider-provided.h"  // for Provided, ProvidedByMacro
#include "tests/cxx/specialization_provider-root.h"  // for RepeatedRoot

***** IWYU_SUMMARY */
