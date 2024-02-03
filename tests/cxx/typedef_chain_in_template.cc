//===--- typedef_chain_in_template.cc - test input file for iwyu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests that if template declares a typedef depending on template argument,
// IWYU follows the typedef chain and uses underlying type that is not a
// typedef depending on template argument.  Usually such typedefs are template
// implementation details and template clients should not use these typedefs
// directly.

#include "tests/cxx/typedef_chain_in_template-d1.h"
#include "tests/cxx/typedef_chain_in_template-d2.h"
#include "tests/cxx/typedef_chain_in_template-d3.h"
#include "tests/cxx/typedef_chain_in_template-d4.h"
#include "tests/cxx/typedef_chain_in_template-d5.h"

void UsageAsWithLibstdcpp() {
  // IWYU: TypedefChainClass needs a declaration
  // IWYU: TypedefChainClass is...*typedef_chain_class.h
  ContainerAsLibstdcpp<TypedefChainClass> c;
  // IWYU: TypedefChainClass is...*typedef_chain_class.h
  c.GetContent().Method();
}

void UsageAsWithLibcpp() {
  // IWYU: TypedefChainClass needs a declaration
  // IWYU: TypedefChainClass is...*typedef_chain_class.h
  ContainerAsLibcpp<TypedefChainClass> c;
  // IWYU: TypedefChainClass is...*typedef_chain_class.h
  c.GetContent().Method();
}

void UsageWithShorterTypedefChain() {
  // IWYU: TypedefChainClass needs a declaration
  // IWYU: TypedefChainClass is...*typedef_chain_class.h
  ContainerShortTypedefChain<TypedefChainClass> c;
  // IWYU: TypedefChainClass is...*typedef_chain_class.h
  c.GetContent1().Method();
  // IWYU: TypedefChainClass is...*typedef_chain_class.h
  c.GetContent2().Method();
}

void UsageWithLongerTypedefChain() {
  // IWYU: TypedefChainClass needs a declaration
  // IWYU: TypedefChainClass is...*typedef_chain_class.h
  ContainerLongTypedefChain<TypedefChainClass> c;
  // IWYU: TypedefChainClass is...*typedef_chain_class.h
  c.GetContent1().Method();
  // IWYU: TypedefChainClass is...*typedef_chain_class.h
  c.GetContent2().Method();
}

void Fn() {
  // IWYU: TypedefChainClass needs a declaration
  // IWYU: TypedefChainClass is...*typedef_chain_class.h
  IdentityStructComplex<TypedefChainClass>::Type isc;
  // IWYU: TypedefChainClass is...*typedef_chain_class.h
  isc.Method();

  // IWYU: TypedefChainClass needs a declaration
  // IWYU: TypedefChainClass is...*typedef_chain_class.h
  IdentityAliasComplex<TypedefChainClass> iac;
  // IWYU: TypedefChainClass is...*typedef_chain_class.h
  iac.Method();

  // IWYU: TypedefChainClass is...*typedef_chain_class.h
  (void)sizeof(TplWithNonProvidedAliased1);
  // IWYU: TypedefChainClass is...*typedef_chain_class.h
  (void)sizeof(TplWithNonProvidedAliased2);
}

/**** IWYU_SUMMARY

tests/cxx/typedef_chain_in_template.cc should add these lines:
#include "tests/cxx/typedef_chain_class.h"

tests/cxx/typedef_chain_in_template.cc should remove these lines:

The full include-list for tests/cxx/typedef_chain_in_template.cc:
#include "tests/cxx/typedef_chain_class.h"  // for TypedefChainClass
#include "tests/cxx/typedef_chain_in_template-d1.h"  // for ContainerAsLibstdcpp
#include "tests/cxx/typedef_chain_in_template-d2.h"  // for ContainerAsLibcpp
#include "tests/cxx/typedef_chain_in_template-d3.h"  // for ContainerShortTypedefChain
#include "tests/cxx/typedef_chain_in_template-d4.h"  // for ContainerLongTypedefChain
#include "tests/cxx/typedef_chain_in_template-d5.h"  // for IdentityAliasComplex, IdentityStructComplex, TplWithNonProvidedAliased1, TplWithNonProvidedAliased2

***** IWYU_SUMMARY */
