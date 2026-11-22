//===--- provided_functions.cc - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -Xiwyu --check_also=tests/cxx/*-direct.h

// Tests re-export policy for external interfaces of classes, i.e. when
// a functions takes a class as its parameter and is declared in the class
// definition header.

#include "tests/cxx/provided_functions-direct.h"

// IWYU: Indirect needs a declaration
Indirect& GetNonProvided();

// IWYU: Indirect needs a declaration
using NonProvidingAlias = Indirect;

template <typename T>
bool Compare(const T& lhs, const T& rhs) {
  return lhs == rhs;
}

template <typename T>
bool CompareWith(T t) {
  ProvidingAlias i;
  return i == t;
}

template <typename U, typename T>
bool CompareHavingUnrelatedParameter(const T& lhs, const T& rhs) {
  return lhs == rhs;
}

void Test() {
  auto& non_provided_by_fn = GetNonProvided();
  auto provided_by_fn = GetProvided();
  extern NonProvidingAlias& non_provided_by_alias;
  extern ProvidingAlias& provided_by_alias;

  // IWYU: swap(Indirect &, Indirect &) is...*-i2.h
  swap(non_provided_by_fn, non_provided_by_fn);
  swap(provided_by_fn, provided_by_fn);
  // IWYU: Indirect is...*-i2.h
  // IWYU: operator==(const Indirect &, const Indirect &) is...*-i2.h
  (void)(non_provided_by_fn == non_provided_by_fn);
  (void)(provided_by_fn == provided_by_fn);

  // IWYU: swap(Indirect &, Indirect &) is...*-i2.h
  swap(non_provided_by_alias, non_provided_by_alias);
  swap(provided_by_alias, provided_by_alias);
  // TODO: Indirect type need not to be complete here.
  // IWYU: Indirect is...*-i2.h
  // IWYU: operator==(const Indirect &, const Indirect &) is...*-i2.h
  (void)(non_provided_by_alias == non_provided_by_alias);
  (void)(provided_by_alias == provided_by_alias);
  // IWYU: Indirect is...*-i2.h
  // IWYU: operator==(const Indirect &, int) is...*-i2.h
  (void)(non_provided_by_alias == 1);
  (void)(provided_by_alias == 1);
  // IWYU: Indirect is...*-i2.h
  // IWYU: operator==(int, const Indirect &) is...*-i2.h
  (void)(1 == non_provided_by_alias);
  (void)(1 == provided_by_alias);

  // IWYU: Indirect is...*-i2.h
  // IWYU: TakeByValue(Indirect) is...*-i2.h
  TakeByValue(non_provided_by_fn);
  TakeByValue(provided_by_fn);
  // IWYU: Indirect is...*-i2.h
  // IWYU: TakeByValue(Indirect) is...*-i2.h
  TakeByValue(non_provided_by_alias);
  TakeByValue(provided_by_alias);

  // Test the "hidden friend".
  // IWYU: Indirect is...*-i2.h
  // IWYU: operator<(const Indirect &, const Indirect &) is...*-i2.h
  (void)(non_provided_by_fn < non_provided_by_fn);
  (void)(provided_by_fn < provided_by_fn);

  IndirectDerived d1, d2;
  // No need to report operator==(const Indirect&, int).
  (void)(d1 == 1);

  // IWYU: operator==(const Indirect &, const Indirect &) is...*-i2.h
  // IWYU: Indirect is...*-i2.h
  Compare(non_provided_by_fn, non_provided_by_fn);
  // TODO: fix this.
  // IWYU: operator==(const Indirect &, const Indirect &) is...*-i2.h
  // IWYU: Indirect is...*-i2.h
  Compare(provided_by_fn, provided_by_fn);
  // IWYU: Indirect is...*-i2.h
  // IWYU: operator==(const Indirect &, const Indirect &) is...*-i2.h
  Compare(non_provided_by_alias, non_provided_by_alias);
  Compare(provided_by_alias, provided_by_alias);

  CompareWith(1);
  // operator== should not be handled as provided just because it is in the same
  // file with the unrelated provided template argument.
  // IWYU: Indirect is...*-i2.h
  // IWYU: operator==(const Indirect &, const Indirect &) is...*-i2.h
  CompareHavingUnrelatedParameter<ProvidingUnrelated>(non_provided_by_fn,
                                                      non_provided_by_fn);

  // IWYU: DifferentFileFunction(Indirect) is...*-i1.h
  DifferentFileFunction(provided_by_fn);
  // IWYU: DifferentFileFunction(Indirect) is...*-i1.h
  DifferentFileFunction(provided_by_alias);
}

/**** IWYU_SUMMARY

tests/cxx/provided_functions.cc should add these lines:
#include "tests/cxx/provided_functions-i1.h"
#include "tests/cxx/provided_functions-i2.h"

tests/cxx/provided_functions.cc should remove these lines:

The full include-list for tests/cxx/provided_functions.cc:
#include "tests/cxx/provided_functions-direct.h"  // for GetProvided, IndirectDerived, ProvidingAlias, ProvidingUnrelated
#include "tests/cxx/provided_functions-i1.h"  // for DifferentFileFunction
#include "tests/cxx/provided_functions-i2.h"  // for Indirect, TakeByValue, operator<, operator==, swap

***** IWYU_SUMMARY */
