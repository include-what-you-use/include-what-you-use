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

class Foo {
  class NoUsage;  // Unnecessary -- defined inline later in the class.
  class NoUsageDefinedOutOfLine;  // Necessary -- part of the public API.
  class NoUsageDefinedOutOfLine;  // Unnecessary -- second fwd declare.
  class UsedAsPtrInMethod;  // Unnecessary -- all uses see the later dfn.
  class UsedFullyInMethod;  // Unnecessary -- all uses see the later dfn.
  class UsedInTypedef;  // Necessary -- this use doesn't see the later dfn.
  class UsedAsPtrArg;  // Necessary -- this use doesn't see the later dfn.
  class UsedAsPtrReturn;  // Necessary -- this use doesn't see the later dfn.

  // If a nested class is used in a body of a method, no preceding
  // declaration/definition is needed.
  void Bar1() {
    UsedAsPtrInMethod* x;
  }
  void Bar2() {
    UsedFullyInMethod x;
  }

  // If a nested class is used in a typedef, a preceding declaration
  // is needed.
  typedef UsedInTypedef UsedInTypedefType;

  // If a nested class is used in a method declaration, a preceding
  // declaration is needed.
  void Bar3(UsedAsPtrArg* p);
  UsedAsPtrReturn* Bar4();

  class NoUsage { };
  class UsedAsPtrInMethod { };
  class UsedFullyInMethod { };
  class UsedAsPtrArg { };
  class UsedAsPtrReturn { };
};

class Foo::NoUsageDefinedOutOfLine {};

// TODO(user,csilvers): Repeat the above with template types.

/**** IWYU_SUMMARY

tests/fwd_decl_nested_class.cc should add these lines:

tests/fwd_decl_nested_class.cc should remove these lines:
- class Foo::NoUsage;  // lines XX-XX
- class Foo::NoUsageDefinedOutOfLine;  // lines XX-XX
- class Foo::UsedAsPtrInMethod;  // lines XX-XX
- class Foo::UsedFullyInMethod;  // lines XX-XX

The full include-list for tests/fwd_decl_nested_class.cc:
class Foo::NoUsageDefinedOutOfLine;  // lines XX-XX
class Foo::UsedAsPtrArg;  // lines XX-XX
class Foo::UsedAsPtrReturn;  // lines XX-XX
class Foo::UsedInTypedef;  // lines XX-XX

***** IWYU_SUMMARY */
