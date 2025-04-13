//===--- memptrs.cc - test input file for iwyu ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests handling pointers to members.

#include "tests/cxx/direct.h"

class Class {};

template <typename T>
class TplUsingInQualifier {
  int T::* p;
};

template <typename T>
class TplUsingInMemberType {
  T Class::* p;
};

// The types involved in member pointer type are forward-declarable in general.

// IWYU: IndirectClass needs a declaration
int IndirectClass::* p1;

// IWYU: IndirectClass needs a declaration
IndirectClass Class::* p2;

// IWYU: IndirectClass needs a declaration
TplUsingInQualifier<IndirectClass> tpl1;

// IWYU: IndirectClass needs a declaration
TplUsingInMemberType<IndirectClass> tpl2;

/**** IWYU_SUMMARY

tests/cxx/memptrs.cc should add these lines:
class IndirectClass;

tests/cxx/memptrs.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/memptrs.cc:
class IndirectClass;

***** IWYU_SUMMARY */
