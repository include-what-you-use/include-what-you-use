//===--- typeid.cc - test input file for iwyu -----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests that IWYU suggests an include instead of a forward declaration for
// expressions used as arguments to typeid(). This is necessary as calling
// typeid() on an incomplete type causes the program to be ill-formed.
// We need both typeid(expr) and typeid(type) calls to cause the necessary files
// to be included, rather than forward declared.

#include "tests/cxx/direct.h"
#include <typeinfo>

// Tests that typeid(expr) triggers an include, rather than a
// forward declaration.
// IWYU: IndirectClass needs a declaration
void GetTypeInfoOfIndirectExpr(const IndirectClass &indirect) {
  // IWYU: IndirectClass is...*indirect.h
  const std::type_info &typeInfo = typeid(indirect);
}

// Tests that typeid(type) triggers an include, rather than a
// forward declaration.
void GetTypeInfoOfIndirectType() {
  // IWYU: IndirectClass is...*indirect.h
  const std::type_info &typeInfo = typeid(IndirectClass);
}

/**** IWYU_SUMMARY

tests/cxx/typeid.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/typeid.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/typeid.cc:
#include <typeinfo>  // for type_info
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
