//===--- no_fwd_decls.cc - test input file for iwyu -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --no_fwd_decls -I .

// Test that passing the --no_fwd_decls switch to IWYU suggests including the
// corresponding header file even when the use is not a full use.
//
// The normal IWYU policy is to minimize the number of #include directives
// without introducing unnecessary coupling.

// The --no_fwd_decls policy is to minimize the number of redeclarations to
// avoid over-loose coupling, which might lead to errors and busy-work when a
// type eventually changes.

#include "tests/cxx/direct.h"
#include "tests/cxx/no_fwd_decls-fwd.h"
#include "tests/cxx/no_fwd_decls-nameonly.h"

// IWYU: IndirectClass is...*indirect.h
IndirectClass* global;

// Existing forward-declares that don't exist anywhere else should stay,
// because we don't know where a declaration would come from otherwise.
class LocalFwd;
void ForwardDeclareUse(const LocalFwd*);

// IWYU must not remove the forward declaration of this class even though its
// definition can be found in the same file but after the use
class LocalFwd {};

// A forward-declare that also exists in an included header can be removed.
// Normally IWYU would optimize for fewer includes, but in --no_fwd_decls mode
// we optimize for fewer redeclarations instead.
class Fwd;
void ForwardDeclareUse(const Fwd&);

// Make sure a forward-declare included from a desired header is not repeated
// here.
bool AreSame(const NameOnly& a, const NameOnly& b) {
  return AddressOf(a) == AddressOf(b);
}

/**** IWYU_SUMMARY

tests/cxx/no_fwd_decls.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/no_fwd_decls.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX
- class Fwd;  // lines XX-XX

The full include-list for tests/cxx/no_fwd_decls.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass
#include "tests/cxx/no_fwd_decls-fwd.h"  // for Fwd
#include "tests/cxx/no_fwd_decls-nameonly.h"  // for AddressOf, NameOnly
class LocalFwd;  // lines XX-XX

***** IWYU_SUMMARY */
