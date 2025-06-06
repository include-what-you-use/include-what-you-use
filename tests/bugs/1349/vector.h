//===--- vector.h - iwyu test ---------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <vector>

template <typename T>
class Vector : private std::vector<T> {
 public:
  using Base = std::vector<T>;

  using Base::Base;
  using Base::begin;
  using Base::end;
};
