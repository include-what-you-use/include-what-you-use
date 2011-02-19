//===--- fwd_decl_nested_class.cc - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that when we forward-declare a nested class, then define the
// class later but out-of-line, we don't delete the forward-declare.
// But when we define the class later in-line, we can delete the
// forward-declare.

class Foo {
  class NestedInline;
  class NestedOutOfLine;

  class NestedInline { };
};

class Foo::NestedOutOfLine {};

/**** IWYU_SUMMARY

tests/fwd_decl_nested_class.cc should add these lines:

tests/fwd_decl_nested_class.cc should remove these lines:
- class Foo::NestedInline;  // lines XX-XX

The full include-list for tests/fwd_decl_nested_class.cc:
class Foo::NestedOutOfLine;  // lines XX-XX

***** IWYU_SUMMARY */
