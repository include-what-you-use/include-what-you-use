//===--- avoids_double_specialization.cc - test input file for iwyu -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This tests a bug I had that was causing a class to get explicitly
// instantiated twice.  iwyu explicitly instantiates typedefed
// classes, but it's supposed to have a check that avoids doing it
// twice (which causes clang to complain).  This check was buggy
// before: the instantiated visitor and the normal visitor were each
// keeping separate values.  Hence this test tests two typedefs of the
// same thing: one in a template and one outside it.

template<class T> struct Foo {
  static int statici;
};
template<class T> int Foo<T>::statici = 0;

template<class T> struct Bar {
  typedef Foo<T> value;
};

// Apparently the bug only evidences when Foo is also implicitly
// instantiated.  I'm not sure why.
Foo<float> implicit_foo;
typedef Foo<float> Baz;

// This implicit instantiation forces evaluation of the typedef, which
// forces another explicitly instantiation of Foo<float>.
Bar<float> implicit_bar;

/**** IWYU_SUMMARY

(tests/cxx/avoids_double_specialization.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
