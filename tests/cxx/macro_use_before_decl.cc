//===--- macro_use_before_decl.cc - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests uses in the body of a macro, of a decl defined in the macro's file
// after the macro definition. The body is compiled where the macro is
// expanded, after the definition, so the file needs nothing for the decl, not
// even a forward-declaration, which for a nested enum isn't even valid C++.

#include "tests/cxx/macro_use_before_decl.h"

#define MAKE_FOO() Foo foo;

class Foo {};

void UseFoo() {
  MAKE_FOO()
}

#define MAKE_FOO_PTR() FooPtr* ptr = nullptr;

class FooPtr {};

void UseFooPtr() {
  MAKE_FOO_PTR()
}

// The class and its method come from one macro, and the method body expands a
// macro defined before it that uses members through 'this'.

#define USE_MEMBERS() \
  int copy = value;   \
  Fixture* self = this;

#define TEST_CLASS(name)   \
  class name : Fixture {   \
   public:                 \
    void Run();            \
  };                       \
  void name::Run()

struct Fixture {
  int value = 0;
};

TEST_CLASS(FixtureTest) {
  USE_MEMBERS()
}

// The macro both defines a struct and uses it.

#define DEFINE_VAR \
  struct Bar {     \
    int x;         \
  };               \
  Bar bar;

DEFINE_VAR

// An X-macro defined before a class lists the enumerators of the class's
// nested enum, and is expanded both to declare them and to use them.

#define FOR_EACH_COLOR(ACTION) \
  ACTION(Red)                  \
  ACTION(Green)

class Palette {
 public:
  enum Color : unsigned {
#define COLOR_ENUMERATOR(C) C,
    FOR_EACH_COLOR(COLOR_ENUMERATOR)
#undef COLOR_ENUMERATOR
  };

  static constexpr Color kColors[] = {
#define COLOR_VALUE(C) C,
      FOR_EACH_COLOR(COLOR_VALUE)
#undef COLOR_VALUE
  };
};

// The X-macro and the enum it fills in are in the associated header, and the
// macro is expanded here.

const Shapes::Shape Shapes::kShapes[] = {
#define SHAPE_VALUE(S) S,
    FOR_EACH_SHAPE(SHAPE_VALUE)
#undef SHAPE_VALUE
};

/**** IWYU_SUMMARY

(tests/cxx/macro_use_before_decl.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
