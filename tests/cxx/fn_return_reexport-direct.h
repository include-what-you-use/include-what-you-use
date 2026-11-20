//===--- fn_return_reexport-direct.h - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/direct.h"
#include "tests/cxx/fn_return_reexport-indirect.h"

// IWYU: IndirectClass is...*indirect.h
IndirectClass GetProvidedIndirectClass();

Indirect GetProvidedIndirect();

// IWYU: IndirectTemplate needs a declaration
// IWYU: IndirectTemplate is...*indirect.h
// IWYU: IndirectClass is...*indirect.h
IndirectTemplate<IndirectClass> GetProvidedTpl();

// IWYU: IndirectClass is...*indirect.h
using ProvidingIndirectClassAlias = IndirectClass;

template <typename T>
struct Container {
  struct Iterator {
    T* operator->() const;
  };
};

inline const auto& hdr_provided = GetProvidedIndirect();

// IWYU: IndirectClass is...*indirect.h
using IndirectClassIterator = Container<IndirectClass>::Iterator;

/**** IWYU_SUMMARY

tests/cxx/fn_return_reexport-direct.h should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/fn_return_reexport-direct.h should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/fn_return_reexport-direct.h:
#include "tests/cxx/fn_return_reexport-indirect.h"  // for Indirect
#include "tests/cxx/indirect.h"  // for IndirectClass, IndirectTemplate

***** IWYU_SUMMARY */
