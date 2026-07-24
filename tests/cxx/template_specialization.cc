//===--- template_specialization.cc - test input file for iwyu ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests that when we instantiate a specialized template, we attribute
// it to the right location.

#include "tests/cxx/template_specialization-d1.h"
#include "tests/cxx/template_specialization-d2.h"
#include "tests/cxx/template_specialization-d3.h"
#include "tests/cxx/direct.h"

template<typename T> class Foo;

// IWYU: Foo<int> is...*template_specialization-i2.h
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


template<typename T>
struct Specialized;

template<>
// IWYU: IndirectClass is...*indirect.h
struct Specialized<int> : IndirectClass {};

// When a specialization requires a forward-declaration of the primary template,
// a fwd-decl in the current file should be preferred to one in the
// otherwise-unused include -d2.h.
// The type alias is needed to trigger a full-use report because there is no
// forward-declaration in the alias-defining file. That full use is then
// recategorized to fwd-decl use because the defn is actually after the alias.
// IWYU: FwdDeclaredTpl needs a declaration
using FwdDeclaredTplSpecAlias = FwdDeclaredTpl<1>;
// IWYU: FwdDeclaredTpl needs a declaration
template <> class FwdDeclaredTpl<1> {};

// IWYU: DefinedBeforeSpec needs a declaration
using DefinedBeforeSpecAlias = DefinedBeforeSpec<1>;
// Define the primary template; no diagnostic here.
template <int> class DefinedBeforeSpec {};

/**** IWYU_SUMMARY

tests/cxx/template_specialization.cc should add these lines:
#include "tests/cxx/indirect.h"
#include "tests/cxx/template_specialization-i1.h"
#include "tests/cxx/template_specialization-i2.h"
template <int> class DefinedBeforeSpec;
template <int> class FwdDeclaredTpl;

tests/cxx/template_specialization.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX
- #include "tests/cxx/template_specialization-d1.h"  // lines XX-XX
- #include "tests/cxx/template_specialization-d2.h"  // lines XX-XX
- #include "tests/cxx/template_specialization-d3.h"  // lines XX-XX
- template <typename T> class Foo;  // lines XX-XX

The full include-list for tests/cxx/template_specialization.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass
#include "tests/cxx/template_specialization-i1.h"  // for Foo
#include "tests/cxx/template_specialization-i2.h"  // for Foo
template <int> class DefinedBeforeSpec;
template <int> class FwdDeclaredTpl;
template <typename T> struct Specialized;  // lines XX-XX+1

***** IWYU_SUMMARY */
