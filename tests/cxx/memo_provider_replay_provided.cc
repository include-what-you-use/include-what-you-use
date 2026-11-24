//===--- memo_provider_replay_provided.cc - test input --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++11

#include "tests/cxx/memo_provider_replay-direct.h"

// When every path provides the explicit instantiation, replaying a
// suppressed use must not report it.
// IWYU: ProvidedByMacro is...*memo_provider_replay-provided.h
// IWYU: Payload is...*memo_provider_replay-payload.h
// IWYU: Root is...*memo_provider_replay-root.h
// IWYU: Payload needs a declaration
// IWYU: Provided is...*memo_provider_replay-provided.h
Root<Provided, ProvidedByMacro, Payload> instance;

/**** IWYU_SUMMARY

tests/cxx/memo_provider_replay_provided.cc should add these lines:
#include "tests/cxx/memo_provider_replay-payload.h"
#include "tests/cxx/memo_provider_replay-provided.h"
#include "tests/cxx/memo_provider_replay-root.h"

tests/cxx/memo_provider_replay_provided.cc should remove these lines:
- #include "tests/cxx/memo_provider_replay-direct.h"  // lines XX-XX

The full include-list for tests/cxx/memo_provider_replay_provided.cc:
#include "tests/cxx/memo_provider_replay-payload.h"  // for Payload
#include "tests/cxx/memo_provider_replay-provided.h"  // for Provided, ProvidedByMacro
#include "tests/cxx/memo_provider_replay-root.h"  // for Root

***** IWYU_SUMMARY */
