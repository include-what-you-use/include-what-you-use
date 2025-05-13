//===--- my_vector.h - iwyu test ------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include <vector>

template<typename T>
class MyVector {
 public:
  using iterator = typename std::vector<T>::iterator;

  iterator begin() { return data_.begin(); }
  iterator end() { return data_.end(); }

 private:
  std::vector<T> data_;
};
