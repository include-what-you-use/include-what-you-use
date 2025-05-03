//===--- num_getter.h - iwyu test -----------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

template <typename T>
struct IntegerWrapper {};

template <typename T>
struct NumGetter {
  int GetNum() {
    return IntegerWrapper<T>::num;
  }
};
