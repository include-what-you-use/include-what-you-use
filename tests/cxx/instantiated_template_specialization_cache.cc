//===--- instantiated_template_specialization_cache.cc - test input -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// A template instantiation can reach the same specialization through
// independent AST paths. IWYU must analyze it once without changing the
// include analysis for either use.
template <class T>
struct Property {
  using type = T;
};

template <class T>
struct UsesPropertyTwice {
  typename Property<T>::type first;
  typename Property<T>::type second;
};

UsesPropertyTwice<int> instance;

/**** IWYU_SUMMARY

(tests/cxx/instantiated_template_specialization_cache.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
