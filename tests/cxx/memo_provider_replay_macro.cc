//===--- memo_provider_replay_macro.cc - test input -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++11

#include "tests/cxx/memo_provider_replay-direct.h"

// The aliases come from a macro defined in another header than the ones
// deciding whether the explicit instantiation is provided.
// IWYU: Box is...*memo_provider_replay-inst.h
// IWYU: Payload is...*memo_provider_replay-payload.h
// IWYU: Root is...*memo_provider_replay-root.h
// IWYU: NonProvidedByMacro is...*memo_provider_replay-nonprovided.h
// IWYU: Payload needs a declaration
// IWYU: ProvidedByMacro is...*memo_provider_replay-provided.h
Root<ProvidedByMacro, NonProvidedByMacro, Payload> instance;

/**** IWYU_SUMMARY

tests/cxx/memo_provider_replay_macro.cc should add these lines:
#include "tests/cxx/memo_provider_replay-inst.h"
#include "tests/cxx/memo_provider_replay-nonprovided.h"
#include "tests/cxx/memo_provider_replay-payload.h"
#include "tests/cxx/memo_provider_replay-provided.h"
#include "tests/cxx/memo_provider_replay-root.h"

tests/cxx/memo_provider_replay_macro.cc should remove these lines:
- #include "tests/cxx/memo_provider_replay-direct.h"  // lines XX-XX

The full include-list for tests/cxx/memo_provider_replay_macro.cc:
#include "tests/cxx/memo_provider_replay-inst.h"  // for Box
#include "tests/cxx/memo_provider_replay-nonprovided.h"  // for NonProvidedByMacro
#include "tests/cxx/memo_provider_replay-payload.h"  // for Payload
#include "tests/cxx/memo_provider_replay-provided.h"  // for ProvidedByMacro
#include "tests/cxx/memo_provider_replay-root.h"  // for Root

***** IWYU_SUMMARY */
