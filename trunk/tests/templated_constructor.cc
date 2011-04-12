//===--- template_args.cc - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that we handle correctly the case of templated constructors.

#include "tests/direct.h"

struct TemplatedConstructor {
  template<typename T> TemplatedConstructor(const T* obj, int dummy) {
    a_ = obj->a;
  }
  int a_;
};

int main() {
  // IWYU: IndirectClass needs a declaration
  IndirectClass* ic = 0;

  // IWYU: IndirectClass is...*indirect.h
  TemplatedConstructor tc(ic, 1);

  // IWYU: IndirectClass is...*indirect.h
  TemplatedConstructor* tcp = new TemplatedConstructor(ic, 1);
}


/**** IWYU_SUMMARY

tests/templated_constructor.cc should add these lines:
#include "tests/indirect.h"

tests/templated_constructor.cc should remove these lines:
- #include "tests/direct.h"  // lines XX-XX

The full include-list for tests/templated_constructor.cc:
#include "tests/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
