//===--- template_args-d3.h - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/direct.h"

template <typename>
struct TplWithMethodWithoutDef;

template <typename>
struct Host;

// The type aliases below provide IndirectClass.

// IWYU: IndirectClass is...*indirect.h
using IndirectClassProviding = IndirectClass;

// IWYU: IndirectClass is...*indirect.h
using ProvidingFunctionAlias = IndirectClass(IndirectClass);

// IWYU: IndirectClass is...*indirect.h
using TplWithMethodWithoutDefProviding = TplWithMethodWithoutDef<IndirectClass>;

template <int>
// IWYU: IndirectClass is...*indirect.h
using ProvidingPtrAlias = IndirectClass*;

// IWYU: IndirectClass is...*indirect.h
using HostProvidingAlias = Host<IndirectClass>;

template <typename>
// IWYU: IndirectClass is...*indirect.h
using HostProvidingAliasTpl = Host<IndirectClass>;

/**** IWYU_SUMMARY

tests/cxx/template_args-d3.h should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/template_args-d3.h should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/template_args-d3.h:
#include "tests/cxx/indirect.h"  // for IndirectClass
template <typename> struct Host;  // lines XX-XX+1
template <typename> struct TplWithMethodWithoutDef;  // lines XX-XX+1

***** IWYU_SUMMARY */
