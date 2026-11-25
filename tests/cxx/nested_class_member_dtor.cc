//===--- nested_class_member_dtor.cc - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that a nested class is not forward-declared when the only use that
// precedes its definition comes from the enclosing class's implicit
// destructor. Destroying the member instantiates Holder<Nested>::~Holder,
// which needs Nested complete. IWYU reports that use at the location of the
// enclosing class, which comes before the nested definition, but it is in a
// destructor and so in complete-class context. IWYU used to suggest the
// invalid 'struct Outer::Nested;'.

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
