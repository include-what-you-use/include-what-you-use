//===--- inheriting_ctor.cc - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -std=c++11

#include "inheriting_ctor-d1.h"

// IWYU: Derived is...*inheriting_ctor-i1.h
void func() { Derived d(1); }

/**** IWYU_SUMMARY

tests/cxx/inheriting_ctor.cc should add these lines:
#include "inheriting_ctor-i1.h"

tests/cxx/inheriting_ctor.cc should remove these lines:
- #include "inheriting_ctor-d1.h"  // lines XX-XX

The full include-list for tests/cxx/inheriting_ctor.cc:
#include "inheriting_ctor-i1.h"  // for Derived

***** IWYU_SUMMARY */
