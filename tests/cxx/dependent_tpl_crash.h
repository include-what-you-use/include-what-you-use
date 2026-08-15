//===--- dependent_tpl_crash.h - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Some cases from dependent_tpl_crash.cc have been duplicated here to test them
// with a different provision policy (only typedefs from headers can provide
// their underlying types).

template <typename>
struct HeaderTpl {
  template <typename U>
  using Alias = typename U::template Nested<int>::Type;
};

struct HeaderDependentFnReturn {
  template <typename T>
  static typename T::template NestedTpl<T> GetNestedTpl() {
    return {};
  }

  using ThisType = HeaderDependentFnReturn;
};

/**** IWYU_SUMMARY

(tests/cxx/dependent_tpl_crash.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
