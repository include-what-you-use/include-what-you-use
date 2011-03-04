//===--- fwd_decl_then_dfn.cc - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// There are a few situations where the language allows a type to be
// forward-declared, but iwyu requires the definition: templates and
// dependent template types, for instance.  In those cases, we run the
// risk that iwyu might not realize a forward-declaration is actually
// needed, if the definition comes after the relevant use.  This tests
// to make sure iwyu does the right thing in those situations.

template<class T> struct Foo;
template<class T> struct SubFoo : public Foo<T> { };

template<class T> struct Bar;
template<class T> struct SubBar : public Foo<typename Bar<T>::type> { };

template<class T> struct Baz;
typedef Baz<int> BazTypedef;

struct Bang;
typedef Bang BangTypedef;

// Now come the definitions, way at the end.

template<class T> struct Foo { };
template<class T> struct Bar { typedef const T type; };
template<class T> struct Baz { };
struct Bang { };

/**** IWYU_SUMMARY

(tests/fwd_decl_then_dfn.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
