//===--- memo_provider_replay_body.cc - test input ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++11

#include "tests/cxx/memo_provider_replay-direct.h"

// The type is traversed in an instantiated function body.
void Fn() {
  // IWYU: Provided is...*memo_provider_replay-provided.h
  // IWYU: NonProvided is...*memo_provider_replay-nonprovided.h
  // IWYU: Payload needs a declaration
  // IWYU: Box is...*memo_provider_replay-inst.h
  // IWYU: Payload is...*memo_provider_replay-payload.h
  // IWYU: UseInBody() is...*memo_provider_replay-root.h
  UseInBody<Provided, NonProvided, Payload>();
}

/**** IWYU_SUMMARY

tests/cxx/memo_provider_replay_body.cc should add these lines:
#include "tests/cxx/memo_provider_replay-inst.h"
#include "tests/cxx/memo_provider_replay-nonprovided.h"
#include "tests/cxx/memo_provider_replay-payload.h"
#include "tests/cxx/memo_provider_replay-provided.h"
#include "tests/cxx/memo_provider_replay-root.h"

tests/cxx/memo_provider_replay_body.cc should remove these lines:
- #include "tests/cxx/memo_provider_replay-direct.h"  // lines XX-XX

The full include-list for tests/cxx/memo_provider_replay_body.cc:
#include "tests/cxx/memo_provider_replay-inst.h"  // for Box
#include "tests/cxx/memo_provider_replay-nonprovided.h"  // for NonProvided
#include "tests/cxx/memo_provider_replay-payload.h"  // for Payload
#include "tests/cxx/memo_provider_replay-provided.h"  // for Provided
#include "tests/cxx/memo_provider_replay-root.h"  // for UseInBody

***** IWYU_SUMMARY */
