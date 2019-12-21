//===--- specialization_needs_decl.cc - test input file for iwyu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that when we specialize a template class, we require a declaration
// of that class.  However, forward-declaring a specialization doesn't
// require a definition.

#include "tests/cxx/specialization_needs_decl-d1.h"

template<> struct TplStruct<int> { };

// Forward-declaring a specialization is the same: we don't require
// the definition.
template<> struct TplStruct<float>;

// Full-using a specialization requires definition of the specialization to be
// included. Not the base template.

// IWYU: Template needs a declaration
int f(Template<int>& t) {
  // IWYU: Template is...*specialization_needs_decl-i1.h
  return t.x;
}

/**** IWYU_SUMMARY

tests/cxx/specialization_needs_decl.cc should add these lines:
#include "tests/cxx/specialization_needs_decl-i1.h"
template <typename T> struct TplStruct;

tests/cxx/specialization_needs_decl.cc should remove these lines:
- #include "tests/cxx/specialization_needs_decl-d1.h"  // lines XX-XX

The full include-list for tests/cxx/specialization_needs_decl.cc:
#include "tests/cxx/specialization_needs_decl-i1.h"  // for Template
template <typename T> struct TplStruct;

***** IWYU_SUMMARY */
