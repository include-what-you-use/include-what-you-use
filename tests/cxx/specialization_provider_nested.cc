//===--- specialization_provider_nested.cc - test input -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++11

#include "tests/cxx/specialization_provider-direct.h"

// A class reached from another class must retain the unprovided descendant's
// explicit-instantiation requirement when its shared specialization is reused.
// IWYU: NonProvided is...*specialization_provider-nonprovided.h
// IWYU: Box is...*specialization_provider-inst.h
// IWYU: Outer is...*specialization_provider-root.h
// IWYU: Payload is...*specialization_provider-payload.h
// IWYU: Payload needs a declaration
// IWYU: Provided is...*specialization_provider-provided.h
Outer<Provided, NonProvided, Payload> instance;

/**** IWYU_SUMMARY

tests/cxx/specialization_provider_nested.cc should add these lines:
#include "tests/cxx/specialization_provider-inst.h"
#include "tests/cxx/specialization_provider-nonprovided.h"
#include "tests/cxx/specialization_provider-payload.h"
#include "tests/cxx/specialization_provider-provided.h"
#include "tests/cxx/specialization_provider-root.h"

tests/cxx/specialization_provider_nested.cc should remove these lines:
- #include "tests/cxx/specialization_provider-direct.h"  // lines XX-XX

The full include-list for tests/cxx/specialization_provider_nested.cc:
#include "tests/cxx/specialization_provider-inst.h"  // for Box
#include "tests/cxx/specialization_provider-nonprovided.h"  // for NonProvided
#include "tests/cxx/specialization_provider-payload.h"  // for Payload
#include "tests/cxx/specialization_provider-provided.h"  // for Provided
#include "tests/cxx/specialization_provider-root.h"  // for Outer

***** IWYU_SUMMARY */
