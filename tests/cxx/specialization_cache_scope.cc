//===--- specialization_cache_scope.cc - test input -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++11

#include "tests/cxx/specialization_provider-direct.h"

// Member<Payload> and Member<Payload*> share one alias expansion, and each
// stores the uses found in it in its own class-member cache entry.
// IWYU: Holder is...*specialization_cache_scope-defs.h
// IWYU: Payload is...*specialization_provider-payload.h
// IWYU: Payload needs a declaration
Holder<Payload> holder;

// Replays the entry for Member<Payload*>, which must not depend on whether
// Member<Payload> reached the shared expansion first.
// IWYU: Reuser is...*specialization_cache_scope-defs.h
// IWYU: Payload is...*specialization_provider-payload.h
// IWYU: Payload needs a declaration
Reuser<Payload> reuser;

/**** IWYU_SUMMARY

tests/cxx/specialization_cache_scope.cc should add these lines:
#include "tests/cxx/specialization_cache_scope-defs.h"
#include "tests/cxx/specialization_provider-payload.h"

tests/cxx/specialization_cache_scope.cc should remove these lines:
- #include "tests/cxx/specialization_provider-direct.h"  // lines XX-XX

The full include-list for tests/cxx/specialization_cache_scope.cc:
#include "tests/cxx/specialization_cache_scope-defs.h"  // for Holder, Reuser
#include "tests/cxx/specialization_provider-payload.h"  // for Payload

***** IWYU_SUMMARY */
