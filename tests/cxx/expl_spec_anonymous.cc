//===--- expl_spec_anonymous.cc - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// https://github.com/include-what-you-use/include-what-you-use/issues/1336
// describes how IWYU would suggest removing anonymous types nested inside class
// template specializations as if they were unused forward-decls:
//
//   tests/cxx/expl_spec_anonymous.cc should add these lines:
//
//   tests/cxx/expl_spec_anonymous.cc should remove these lines:
//   -   // lines 33-33
//   -   // lines 37-37
//   -   // lines 43-43
//   -   // lines 47-47
//
//   The full include-list for tests/cxx/expl_spec_anonymous.cc:
//   ---
//
// This was fixed in 69e61ca8ed06045bc40f7d909e4199a914dd58b2, but the isolated
// repro case never made it into the tree.

// A base template.
template <class T>
class Class {};

// A specialization containing anonymous types.
template <>
class Class<int> {
  union {
    float a;
    int b;
  };
  struct {
    float c;
    int d;
  };

  // Bonus: unnamed types declaring named members.
  union {
    float a;
    int b;
  } x;
  struct {
    float a;
    int b;
  } y;
};

/**** IWYU_SUMMARY

(tests/cxx/expl_spec_anonymous.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
