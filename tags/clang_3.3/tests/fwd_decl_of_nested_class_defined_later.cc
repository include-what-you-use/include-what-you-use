//===--- fwd_decl_of_nested_class_defined_later.cc - test input file for iwyu ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This tests a particular situation which until recently IWYU did not
// handle correctly. In certain cases, it would have unnecessarily recommended
// a forward declaration *outside* of a class definition which really
// belonged inside the class definition.

#include <utility>

class Foo {
 private:
  int Bar() {
    // The bug was that in the AST tree that IWYU generates, a
    // CXXMethodDecl node representing the pair destructor declaration
    // intervened in the chain from the node corresponding to the use
    // of Baz up to the CXXMethodDecl node for Bar. This caused
    // IsNodeInsideCXXMethodBody to return false for the node
    // corresponding to the use of Baz.
    // The fix was to not stop at the destructor declaration when
    // walking up the tree, since the context was not the body of the
    // destructor.
    return std::make_pair(Baz(), 0).second;
  }

  struct Baz {
    int value_;
  };
};

/**** IWYU_SUMMARY

(tests/fwd_decl_of_nested_class_defined_later.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
