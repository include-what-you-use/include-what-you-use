//===--- fwd_decl_nested_class.cc - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Various tests of the handling of the forward declaration of nested
// classes. For some uses the forward declaration is needed, for
// others it isn't.

template<typename T> void TplFn() { T t; (void)t; }

class Foo {
  class NoUsage;  // Unnecessary -- defined inline later in the class.
  class NoUsageDefinedOutOfLine;  // Necessary -- part of the public API.
  class NoUsageDefinedOutOfLine;  // Unnecessary -- second fwd declare.
  class UsedAsPtrInMethod;  // Unnecessary -- all uses see the later dfn.
  class UsedFullyInMethod;  // Unnecessary -- all uses see the later dfn.
  class UsedFullyInInitializer;  // Unnecessary -- same as in a method.
  class UsedImplicitlyInInitializer;  // Unnecessary -- same as explicit use.
  class UsedInTypedef;  // Necessary -- this use doesn't see the later dfn.
  class UsedAsPtrArg;  // Necessary -- this use doesn't see the later dfn.
  class UsedAsPtrReturn;  // Necessary -- this use doesn't see the later dfn.
  class UsedAsPtrMember;  // Necessary -- this use doesn't see the later dfn.

  Foo() : init_(UsedFullyInInitializer()) { }
  ~Foo() { }

  // If a nested class is used in a body of a method, no preceding
  // declaration/definition is needed.
  void Bar1() {
    UsedAsPtrInMethod* x;
  }
  void Bar2() {
    UsedFullyInMethod x;
    UsedFullyInMethodNotForwardDeclared y;
    // Make sure we don't have trouble saying we're inside a method
    // even when templates get in the way.
    TplFn<UsedFullyInMethodNotForwardDeclared>();
  }

  // If a nested class is used in a typedef, a preceding declaration
  // is needed.
  typedef UsedInTypedef UsedInTypedefType;

  // If a nested class is used in a method declaration, a preceding
  // declaration is needed.
  void Bar3(UsedAsPtrArg* p);
  UsedAsPtrReturn* Bar4();

  UsedAsPtrMember* x_;

  class NoUsage { };
  class UsedAsPtrInMethod { };
  class UsedFullyInMethod { };
  class UsedFullyInInitializer { };
  class UsedImplicitlyInInitializer { };
  class UsedImplicitlyInInitializerNeverDeclared { };
  class UsedAsPtrArg { };
  class UsedAsPtrReturn { };
  class UsedAsPtrMember { };
  struct UsedFullyInMethodNotForwardDeclared { };

  UsedFullyInInitializer init_;
  UsedImplicitlyInInitializer implicit_;
  UsedImplicitlyInInitializerNeverDeclared implicit_never_declared_;
  UsedImplicitlyInInitializerNeverDeclared* implicit_never_declared_ptr_;
};

class Foo::NoUsageDefinedOutOfLine {};

// Now do the same thing again, but the nested classes are all templated.

class Outer {
  template<typename T> class NoUsage;  // Unnecessary
  template<typename T> class NoUsageDefinedOutOfLine;  // Necessary
  template<typename T> class NoUsageDefinedOutOfLine;  // Unnecessary
  template<typename T> class UsedAsPtrInMethod;  // Unnecessary
  template<typename T> class UsedFullyInMethod;  // Unnecessary
  template<typename T> class UsedFullyInInitializer;  // Unnecessary
  template<typename T> class UsedImplicitlyInInitializer;  // Unnecessary
  template<typename T> class UsedInTypedef;  // Necessary
  template<typename T> class UsedAsPtrArg;  // Necessary
  template<typename T> class UsedAsPtrReturn;  // Necessary
  template<typename T> class UsedAsPtrMember;  // Necessary

  Outer() : init_(UsedFullyInInitializer<int>()) { }
  ~Outer() { }

  // If a nested class is used in a body of a method, no preceding
  // declaration/definition is needed.
  void Bar1() {
    UsedAsPtrInMethod<int>* x;
  }
  void Bar2() {
    UsedFullyInMethod<int> x;
    UsedFullyInMethodNotForwardDeclared<int> y;
    TplFn<UsedFullyInMethodNotForwardDeclared<int> >();
  }

  // If a nested class is used in a typedef, a preceding declaration
  // is needed.
  typedef UsedInTypedef<int> UsedInTypedefType;

  // If a nested class is used in a method declaration, a preceding
  // declaration is needed.
  void Bar3(UsedAsPtrArg<int>* p);
  UsedAsPtrReturn<int>* Bar4();

  UsedAsPtrMember<int>* x_;

  template<typename T> class NoUsage { };
  template<typename T> class UsedAsPtrInMethod { };
  template<typename T> class UsedFullyInMethod { };
  template<typename T> class UsedFullyInInitializer { };
  template<typename T> class UsedImplicitlyInInitializer { };
  template<typename T> class UsedImplicitlyInInitializerNeverDeclared { };
  template<typename T> class UsedAsPtrArg { };
  template<typename T> class UsedAsPtrReturn { };
  template<typename T> class UsedAsPtrMember { };
  template<typename T> class UsedFullyInMethodNotForwardDeclared { };

  UsedFullyInInitializer<int> init_;
  UsedImplicitlyInInitializer<int> implicit_;
  UsedImplicitlyInInitializerNeverDeclared<int> implicit_never_declared_;
  UsedImplicitlyInInitializerNeverDeclared<int>* implicit_never_declared_ptr_;
};

template<typename T> class Outer::NoUsageDefinedOutOfLine {};

/**** IWYU_SUMMARY

tests/fwd_decl_nested_class.cc should add these lines:

tests/fwd_decl_nested_class.cc should remove these lines:
- class Foo::NoUsage;  // lines XX-XX
- class Foo::NoUsageDefinedOutOfLine;  // lines XX-XX
- class Foo::UsedAsPtrInMethod;  // lines XX-XX
- class Foo::UsedFullyInInitializer;  // lines XX-XX
- class Foo::UsedFullyInMethod;  // lines XX-XX
- class Foo::UsedImplicitlyInInitializer;  // lines XX-XX
- template <typename T> class Outer::NoUsage;  // lines XX-XX
- template <typename T> class Outer::NoUsageDefinedOutOfLine;  // lines XX-XX
- template <typename T> class Outer::UsedAsPtrInMethod;  // lines XX-XX
- template <typename T> class Outer::UsedFullyInInitializer;  // lines XX-XX
- template <typename T> class Outer::UsedFullyInMethod;  // lines XX-XX
- template <typename T> class Outer::UsedImplicitlyInInitializer;  // lines XX-XX

The full include-list for tests/fwd_decl_nested_class.cc:
class Foo::NoUsageDefinedOutOfLine;  // lines XX-XX
class Foo::UsedAsPtrArg;  // lines XX-XX
class Foo::UsedAsPtrMember;  // lines XX-XX
class Foo::UsedAsPtrReturn;  // lines XX-XX
class Foo::UsedInTypedef;  // lines XX-XX
template <typename T> class Outer::NoUsageDefinedOutOfLine;  // lines XX-XX
template <typename T> class Outer::UsedAsPtrArg;  // lines XX-XX
template <typename T> class Outer::UsedAsPtrMember;  // lines XX-XX
template <typename T> class Outer::UsedAsPtrReturn;  // lines XX-XX
template <typename T> class Outer::UsedInTypedef;  // lines XX-XX

***** IWYU_SUMMARY */
