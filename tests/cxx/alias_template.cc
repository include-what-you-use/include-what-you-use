//===--- alias_template.cc - test input file for iwyu ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -std=c++11 -I .

// Tests alias templates.  Does not test type aliases.

#include "tests/cxx/alias_template.h"
#include "tests/cxx/direct.h"

template <typename T>
using Identity = T;

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
Identity<IndirectClass> ic;
// IWYU: IndirectClass is...*indirect.h
constexpr auto s = sizeof(ic);
// IWYU: IndirectClass needs a declaration
Identity<IndirectClass>* pic = nullptr;

Identity<Providing> type_is_provided_by_arg_1;

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
HeaderIdentity<IndirectClass> hic;
// IWYU: IndirectClass is...*indirect.h
constexpr auto hs = sizeof(hic);
// IWYU: IndirectClass needs a declaration
HeaderIdentity<IndirectClass>* hpic = nullptr;

HeaderIdentity<Providing> type_is_provided_by_arg_2;

template<class T> struct FullUseTemplateArgInSizeof {
  char argument[sizeof(T)];
};

// Test that we go through alias template and handle aliased template
// specialization.
template<class T> using Alias = FullUseTemplateArgInSizeof<T>;
// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
Alias<IndirectClass> alias;
// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
HeaderAlias<IndirectClass> headerAlias;

// Test following through entire chain of aliases.
template<class T> using AliasChain1 = FullUseTemplateArgInSizeof<T>;
template<class T> using AliasChain2 = AliasChain1<T>;
// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
AliasChain2<IndirectClass> aliasChain;
// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
HeaderAliasChain2<IndirectClass> headerAliasChain;

// Test the case when aliased type isn't a template specialization.
template<class T> using Pointer = T*;
Pointer<int> intPtr;
HeaderPointer<int> headerIntPtr;

template <class T>
struct FullUseTemplateArgAsVar {
  T t;
};

// Test the used class being nested deeper in the alias
template <typename T>
using AliasNested = FullUseTemplateArgAsVar<FullUseTemplateArgAsVar<T>>;

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
AliasNested<IndirectClass> aliasNested;

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
HeaderAliasNested<IndirectClass> headerAliasNested;

template <typename T>
using AliasNested2 = FullUseTemplateArgInSizeof<FullUseTemplateArgInSizeof<T>>;
// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
AliasNested2<IndirectClass> aliasNested2;
// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
HeaderAliasNested2<IndirectClass> headerAliasNested2;

template <typename T>
using UsingArgInternals = decltype(T::a);

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
UsingArgInternals<IndirectClass> aliased_int;
// Full type is needed even in fwd-decl context.
// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
UsingArgInternals<IndirectClass>* p_int = nullptr;

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
HeaderUsingArgInternals<IndirectClass> header_aliased_int;
// Full type is needed even in fwd-decl context.
// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
HeaderUsingArgInternals<IndirectClass>* header_p_int = nullptr;

template <typename T>
struct TplWithUsingArgInternals {
  UsingArgInternals<T>* p = nullptr;
};

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
TplWithUsingArgInternals<IndirectClass> twuai;

template <typename T>
struct TplWithHeaderUsingArgInternals {
  HeaderUsingArgInternals<T>* p = nullptr;
};

// IWYU: IndirectClass needs a declaration
// IWYU: IndirectClass is...*indirect.h
TplWithHeaderUsingArgInternals<IndirectClass> twhuai;

template <typename T>
void TakeByCopyInTplFn(T);

void TestProvisionByTplArg() {
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  Identity<IndirectClass> iic;
  // IWYU: IndirectClass is...*indirect.h
  TakeByCopyInTplFn(iic);
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  HeaderIdentity<IndirectClass> hiic;
  // IWYU: IndirectClass is...*indirect.h
  TakeByCopyInTplFn(hiic);

  // Test collecting provided types for SourceOrTargetTypeIsProvided function.
  Identity<Providing> ip;
  TakeByCopyInTplFn(ip);
  HeaderIdentity<Providing> hip;
  TakeByCopyInTplFn(hip);
}

/**** IWYU_SUMMARY

tests/cxx/alias_template.cc should add these lines:

tests/cxx/alias_template.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/alias_template.cc:
#include "tests/cxx/alias_template.h"

***** IWYU_SUMMARY */
