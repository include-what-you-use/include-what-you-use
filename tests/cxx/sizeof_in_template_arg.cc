//===--- sizeof_in_template_arg.cc - test input file for iwyu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

#include "tests/cxx/direct.h"

// This verifies that using sizeof(...) means that the argument of sizeof
// doesn't count as being in a forward-declare context.  In particular, when
// it's used as a template argument.

template <unsigned long Size>
struct Storage {
  char storage[Size];
};

template <typename T>
struct Public {
  Storage<sizeof(T)> storage;
};

// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
Public<IndirectClass> p;

/**** IWYU_SUMMARY

tests/cxx/sizeof_in_template_arg.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/sizeof_in_template_arg.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/sizeof_in_template_arg.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
