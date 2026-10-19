//===--- nested_class_member_dtor.cc - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that a nested class is not forward-declared because of the enclosing
// class's implicit destructor. Destroying the member instantiates
// Holder<Nested>::~Holder, and the name of that destructor refers to
// Holder<Nested>. The name is not a use, so it must not report Nested at the
// location of Outer, which comes before the nested definition and would need
// the invalid 'struct Outer::Nested;'.

template <typename T>
struct Holder {
  ~Holder() {
  }
  T t;
};

struct Outer {
  struct Nested {};
  Holder<Nested> h;
};

/**** IWYU_SUMMARY

(tests/cxx/nested_class_member_dtor.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
