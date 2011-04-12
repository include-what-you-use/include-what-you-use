//===--- delete_in_template.cc - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// C++ has the following rule: when you instantiate a template class,
// and any virtual method in that class calls delete, you had better
// provide a definition for the type being deleted, even if you never
// call the virtual method.  While it's not a language error, g++ and
// clang both warn on it, because the destructors of the deleted type
// won't be run if the virtual method is called via a base class.  (I
// think?)  iwyu should be careful to provide full types in that case.

#include "tests/direct.h"

// TODO(csilvers): can take this out when TODOs below are removed.
// It's in here only to make the IWYU_SUMMARY look like it should.
// IWYU: IndirectClass is...*indirect.h
IndirectClass unused;

template <typename T> struct Deleter {
  Deleter(T* t) : t_(t) {}
  virtual void Delete() { delete t_; }
  static int NothingToDoWithDelete() { return 1; };
  T* t_;
};

// Note we require the full type even though we don't call Delete.
// TODO(csilvers): // IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
Deleter<IndirectClass> d(0);

// IWYU: IndirectClass needs a declaration
Deleter<IndirectClass>* pd
// Another way to instantiate a template, also requirs the full type.
// TODO(csilvers): // IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
    = new Deleter<IndirectClass>(0);

// This also instantiates the template, and thus requires the deleted-type.
// TODO(csilvers): IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
int id = Deleter<IndirectClass>::NothingToDoWithDelete();


template <typename T> struct NonvirtualDeleter {
  NonvirtualDeleter(T* t) : t_(t) {}
  void Delete() { delete t_; }
  static int NothingToDoWithDelete() { return 1; };
  T* t_;
};

// None of these should require the full type.
// IWYU: IndirectClass needs a declaration
NonvirtualDeleter<IndirectClass> nd(0);

// IWYU: IndirectClass needs a declaration
NonvirtualDeleter<IndirectClass>* pnd
// IWYU: IndirectClass needs a declaration
    = new NonvirtualDeleter<IndirectClass>(0);

// IWYU: IndirectClass needs a declaration
int ind = NonvirtualDeleter<IndirectClass>::NothingToDoWithDelete();


/**** IWYU_SUMMARY

tests/delete_in_template.cc should add these lines:
#include "tests/indirect.h"

tests/delete_in_template.cc should remove these lines:
- #include "tests/direct.h"  // lines XX-XX

The full include-list for tests/delete_in_template.cc:
#include "tests/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
