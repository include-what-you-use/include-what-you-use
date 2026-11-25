//===--- specialization_provider_macro.cc - test input --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++11

#include "tests/cxx/specialization_provider-direct.h"

// The aliases come from a macro defined in another header than the ones
// deciding whether the explicit instantiation is provided.
// IWYU: Box is...*specialization_provider-inst.h
// IWYU: Payload is...*specialization_provider-payload.h
// IWYU: RepeatedRoot is...*specialization_provider-root.h
// IWYU: NonProvidedByMacro is...*specialization_provider-nonprovided.h
// IWYU: Payload needs a declaration
// IWYU: ProvidedByMacro is...*specialization_provider-provided.h
RepeatedRoot<ProvidedByMacro, NonProvidedByMacro, Payload> instance;

/**** IWYU_SUMMARY

tests/cxx/specialization_provider_macro.cc should add these lines:
#include "tests/cxx/specialization_provider-inst.h"
#include "tests/cxx/specialization_provider-nonprovided.h"
#include "tests/cxx/specialization_provider-payload.h"
#include "tests/cxx/specialization_provider-provided.h"
#include "tests/cxx/specialization_provider-root.h"

tests/cxx/specialization_provider_macro.cc should remove these lines:
- #include "tests/cxx/specialization_provider-direct.h"  // lines XX-XX

The full include-list for tests/cxx/specialization_provider_macro.cc:
#include "tests/cxx/specialization_provider-inst.h"  // for Box
#include "tests/cxx/specialization_provider-nonprovided.h"  // for NonProvidedByMacro
#include "tests/cxx/specialization_provider-payload.h"  // for Payload
#include "tests/cxx/specialization_provider-provided.h"  // for ProvidedByMacro
#include "tests/cxx/specialization_provider-root.h"  // for RepeatedRoot

***** IWYU_SUMMARY */
