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
#include "tests/cxx/typedef_chain_class.h"
// Unused include to trigger IWYU summary telling what symbols are used from
// every file.
#include "tests/cxx/direct.h"

void UsageAsWithLibstdcpp() {
  ContainerAsLibstdcpp<TypedefChainClass> c;
  c.GetContent().Method();
}

void UsageAsWithLibcpp() {
  ContainerAsLibcpp<TypedefChainClass> c;
  c.GetContent().Method();
}

void UsageWithShorterTypedefChain() {
  ContainerShortTypedefChain<TypedefChainClass> c;
  c.GetContent1().Method();
  c.GetContent2().Method();
}

void UsageWithLongerTypedefChain() {
  ContainerLongTypedefChain<TypedefChainClass> c;
  c.GetContent1().Method();
  c.GetContent2().Method();
}

/**** IWYU_SUMMARY

tests/cxx/typedef_chain_in_template.cc should add these lines:

tests/cxx/typedef_chain_in_template.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/typedef_chain_in_template.cc:
#include "tests/cxx/typedef_chain_class.h"  // for TypedefChainClass
#include "tests/cxx/typedef_chain_in_template-d1.h"  // for ContainerAsLibstdcpp
#include "tests/cxx/typedef_chain_in_template-d2.h"  // for ContainerAsLibcpp
#include "tests/cxx/typedef_chain_in_template-d3.h"  // for ContainerShortTypedefChain
#include "tests/cxx/typedef_chain_in_template-d4.h"  // for ContainerLongTypedefChain

***** IWYU_SUMMARY */
