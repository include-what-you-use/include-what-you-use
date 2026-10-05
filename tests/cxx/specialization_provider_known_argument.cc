//===--- specialization_provider_known_argument.cc - test input -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++11

#include "tests/cxx/specialization_provider-direct.h"

// A written Box argument independently requires its declaration and explicit
// instantiation, even when provider-dependent traversal skips a reported use.
// This is an output control: the lost ordinary use requires instrumentation
// to distinguish because the independent use preserves the final includes.
// IWYU: Both is...*specialization_provider-root.h
// IWYU: Box is...*specialization_provider-defs.h
// IWYU: Box is...*specialization_provider-inst.h
// IWYU: Payload is...*specialization_provider-payload.h
// IWYU: Provided is...*specialization_provider-provided.h
// IWYU: NonProvided is...*specialization_provider-nonprovided.h
// IWYU: Box needs a declaration
// IWYU: Payload needs a declaration
int instance = sizeof(Both<Provided, NonProvided, Box<Payload>>);

/**** IWYU_SUMMARY

tests/cxx/specialization_provider_known_argument.cc should add these lines:
#include "tests/cxx/specialization_provider-defs.h"
#include "tests/cxx/specialization_provider-inst.h"
#include "tests/cxx/specialization_provider-nonprovided.h"
#include "tests/cxx/specialization_provider-payload.h"
#include "tests/cxx/specialization_provider-provided.h"
#include "tests/cxx/specialization_provider-root.h"

tests/cxx/specialization_provider_known_argument.cc should remove these lines:
- #include "tests/cxx/specialization_provider-direct.h"  // lines XX-XX

The full include-list for tests/cxx/specialization_provider_known_argument.cc:
#include "tests/cxx/specialization_provider-defs.h"  // for Box
#include "tests/cxx/specialization_provider-inst.h"  // for Box
#include "tests/cxx/specialization_provider-nonprovided.h"  // for NonProvided
#include "tests/cxx/specialization_provider-payload.h"  // for Payload
#include "tests/cxx/specialization_provider-provided.h"  // for Provided
#include "tests/cxx/specialization_provider-root.h"  // for Both

***** IWYU_SUMMARY */
