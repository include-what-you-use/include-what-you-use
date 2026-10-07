//===--- tpl_spec_cast.cc - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests that a full use of a class template specialization inside an
// instantiated template, like a cast to a pointer to it or a member access
// through such a pointer, reports the template arguments the specialization
// holds by value.

#include "tests/cxx/direct.h"

struct Base {};

template <typename T>
struct Box : Base {
  T obj;
};

template <typename T>
class Ptr {
 public:
  void Touch() const {
    (void)static_cast<Box<T>*>(p);
  }
  T* Get() const {
    return &static_cast<Box<T>*>(p)->obj;
  }
  T* Member() const {
    return &b->obj;
  }

 private:
  Base* p = nullptr;
  Box<T>* b = nullptr;
};

void Fn() {
  // IWYU: IndirectClass needs a declaration
  Ptr<IndirectClass> ptr;
  // IWYU: IndirectClass is...*indirect.h
  ptr.Touch();
  // IWYU: IndirectClass is...*indirect.h
  ptr.Get();
  // IWYU: IndirectClass is...*indirect.h
  ptr.Member();
}

/**** IWYU_SUMMARY

tests/cxx/tpl_spec_cast.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/tpl_spec_cast.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/tpl_spec_cast.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
