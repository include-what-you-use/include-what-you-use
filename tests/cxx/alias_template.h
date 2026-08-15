//===--- alias_template.h - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/direct.h"

// Some declarations from alias_template.cc have been duplicated here to test
// alias templates with a different provision policy (only header declarations
// can provide).

template <typename T>
using HeaderIdentity = T;

// IWYU: IndirectClass is...*indirect.h
using Providing = IndirectClass;

template <class T>
struct HeaderFullUseTemplateArgInSizeof {
  char argument[sizeof(T)];
};

// Test that we go through alias template and handle aliased template
// specialization.
template <class T>
using HeaderAlias = HeaderFullUseTemplateArgInSizeof<T>;

// Test following through entire chain of aliases.
template <class T>
using HeaderAliasChain1 = HeaderFullUseTemplateArgInSizeof<T>;
template <class T>
using HeaderAliasChain2 = HeaderAliasChain1<T>;

// Test the case when aliased type isn't a template specialization.
template <class T>
using HeaderPointer = T*;

template <class T>
struct HeaderFullUseTemplateArgAsVar {
  T t;
};

// Test the used class being nested deeper in the alias
template <typename T>
using HeaderAliasNested =
    HeaderFullUseTemplateArgAsVar<HeaderFullUseTemplateArgAsVar<T>>;

template <typename T>
using HeaderAliasNested2 =
    HeaderFullUseTemplateArgInSizeof<HeaderFullUseTemplateArgInSizeof<T>>;

template <typename T>
using HeaderUsingArgInternals = decltype(T::a);

/**** IWYU_SUMMARY

tests/cxx/alias_template.h should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/alias_template.h should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/alias_template.h:
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
