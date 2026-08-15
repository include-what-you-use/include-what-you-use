//===--- fwd_decl_nested_class.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Some cases from fwd_decl_nested_class.cc have been duplicated here to test
// them with a different provision policy (only typedefs from headers can
// provide their underlying types).

class HeaderFoo {
  class UsedInTypedef;

  // If a nested class is used in a typedef, a preceding declaration
  // is needed.
  typedef UsedInTypedef UsedInTypedefType;
};

class HeaderOuter {
  template <typename T>
  class UsedInTypedef;  // Necessary.

  // If a nested class is used in a typedef, a preceding declaration
  // is needed.
  typedef UsedInTypedef<int> UsedInTypedefType;
};

template <class T>
class HeaderContainer {
  class UsedInTypedef;  // Necessary.

  // If a nested class is used in a typedef, a preceding declaration
  // is needed.
  typedef UsedInTypedef UsedInTypedefType;
};

/**** IWYU_SUMMARY

(tests/cxx/fwd_decl_nested_class.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
