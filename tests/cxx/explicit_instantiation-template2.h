//===--- explicit_instantiation-template2.h - test input file for iwyu ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

template <typename T>
class ClassWithUsingMethod2 {
  void Fn() {
    T t;
  }
};
