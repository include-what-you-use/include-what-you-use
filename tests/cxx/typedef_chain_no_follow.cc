//===--- typedef_chain_no_follow.cc - test input file for iwyu ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests cases when IWYU should not follow typedef chain and should not suggest
// to include a file for underlying typedef type.

#include "tests/cxx/typedef_chain_no_follow-d1.h"
#include "tests/cxx/typedef_chain_no_follow-d2.h"
#include "tests/cxx/typedef_chain_no_follow-d3.h"
#include "tests/cxx/typedef_chain_no_follow-d4.h"
// Unused include to trigger IWYU summary telling what symbols are used from
// every file.
#include "tests/cxx/direct.h"

void TypedefDeclaredInGlobalNamespace() {
  TypedefChainTypedef tct;
  tct.Method();

  ProvidingWithAliasTpl pwat;
  pwat.Method();

  ProvidingWithStructTpl pwst;
  pwst.Method();
}

// Tests how we handle a typedef declared in a class.  Main purpose is to make
// sure we handle typedefs in templates the same way as typedefs in classes
// when typedef does not depend on template argument.
void TypedefDeclaredInClass() {
  NonContainer1 nc;
  nc.GetTypedefChainClass().Method();
}

// Tests that we don't follow typedef chain and don't suggest to use
// TypedefChainClass directly.
void TypedefDeclaredInTemplate() {
  NonContainer2<int> nc;
  nc.GetTypedefChainClass().Method();
}

/**** IWYU_SUMMARY

tests/cxx/typedef_chain_no_follow.cc should add these lines:

tests/cxx/typedef_chain_no_follow.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/typedef_chain_no_follow.cc:
#include "tests/cxx/typedef_chain_no_follow-d1.h"  // for TypedefChainTypedef
#include "tests/cxx/typedef_chain_no_follow-d2.h"  // for NonContainer1
#include "tests/cxx/typedef_chain_no_follow-d3.h"  // for NonContainer2
#include "tests/cxx/typedef_chain_no_follow-d4.h"  // for ProvidingWithAliasTpl, ProvidingWithStructTpl

***** IWYU_SUMMARY */
