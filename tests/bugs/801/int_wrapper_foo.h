//===--- int_wrapper_foo.h - iwyu test ------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

template <>
struct IntegerWrapper<Foo> {
  static constexpr int num = Foo::num;
};
