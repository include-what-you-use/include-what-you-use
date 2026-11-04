//===--- instantiated_template_specialization_cache-indirect.h - test -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INSTANTIATED_TEMPLATE_SPECIALIZATION_CACHE_INDIRECT_H_
#define INSTANTIATED_TEMPLATE_SPECIALIZATION_CACHE_INDIRECT_H_

template <class Left, class Right>
struct Pair {
  Left left;
  Right right;
};

template <class T>
using Duplicate = Pair<T, T>;

template <class T>
struct Holder {
  Duplicate<Duplicate<Duplicate<Duplicate<Duplicate<Duplicate<
      Duplicate<Duplicate<Duplicate<Duplicate<Duplicate<Duplicate<T>>>>>>>>>>>>
      value;
};

#endif  // INSTANTIATED_TEMPLATE_SPECIALIZATION_CACHE_INDIRECT_H_
