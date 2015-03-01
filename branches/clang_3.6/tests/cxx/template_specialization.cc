//===--- template_specialization.cc - test input file for iwyu ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that when we instantiate a specialized template, we attribute
// it to the right location.

#include "tests/cxx/template_specialization-d1.h"

template<typename T> class Foo;

// IWYU: Foo is...*template_specialization-i2.h
Foo<int> foo;
// Even though Foo<int> has a specialization, it doesn't matter
// because forward-declaring is ok.
Foo<int>* foo_ptr;

// Check we do the right thing when we're a template template arg, too.
// IWYU: Foo is...*template_specialization-i1.h
template <template<typename U> class T = Foo> struct TplTplStruct {
  // TODO(csilvers): it's probably correct to say we need
  // template_specialization-i1.h here, because if someone doesn't
  // specify any tpl args when instantiating us, we'll create a
  // Foo<int>, so as tpl authors we're responsible for the definition
  // of Foo<int>.  This is pretty unlikely to happen in practice.
  T<int> u;
};

TplTplStruct<> tts;
// TODO(csilvers): This should find the -i2.h location (for Foo<int>),
// not -i1.h (for Foo<T>).
// IWYU: Foo is...*template_specialization-i1.h
TplTplStruct<Foo> tts2;


/**** IWYU_SUMMARY

tests/cxx/template_specialization.cc should add these lines:
#include "tests/cxx/template_specialization-i1.h"
#include "tests/cxx/template_specialization-i2.h"

tests/cxx/template_specialization.cc should remove these lines:
- #include "tests/cxx/template_specialization-d1.h"  // lines XX-XX
- template <typename T> class Foo;  // lines XX-XX

The full include-list for tests/cxx/template_specialization.cc:
#include "tests/cxx/template_specialization-i1.h"  // for Foo
#include "tests/cxx/template_specialization-i2.h"  // for Foo

***** IWYU_SUMMARY */
