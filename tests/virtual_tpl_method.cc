//===--- virtual_tpl_method.cc - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Normally, C++ only instantiates methods on template classes when
// the methods are called.  But for virtual methods, they're
// instantiated when the key method for the template class is
// instantiated.  Since template classes rarely have key methods, that
// means they're instantiated when the class is instantiated.  We test
// that iwyu correctly handles that situation.

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
// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
Deleter<IndirectClass> d(0);

// IWYU: IndirectClass needs a declaration
Deleter<IndirectClass>* pd
// Another way to instantiate a template, also requirs the full type.
// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
    = new Deleter<IndirectClass>(0);

// This also instantiates the template, and thus requires the deleted-type.
// IWYU: IndirectClass is...*indirect.h
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


template <typename T> struct NonTplArgDeleter {
  virtual void Delete() {
    // IWYU: IndirectClass needs a declaration
    IndirectClass* ic = 0;
    // IWYU: IndirectClass is...*indirect.h
    delete ic;
  }
};

// These should not require the full type, for the normal reason that
// they're not a template arg.
NonTplArgDeleter<int> ntad;



/**** IWYU_SUMMARY

tests/virtual_tpl_method.cc should add these lines:
#include "tests/indirect.h"

tests/virtual_tpl_method.cc should remove these lines:
- #include "tests/direct.h"  // lines XX-XX

The full include-list for tests/virtual_tpl_method.cc:
#include "tests/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
