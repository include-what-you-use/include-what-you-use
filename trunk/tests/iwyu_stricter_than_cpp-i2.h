//===--- iwyu_stricter_than_cpp-i2.h - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

struct IndirectStruct2 {
  IndirectStruct2(int) {}
  int a;
};

template <typename T> struct TplIndirectStruct2 { TplIndirectStruct2(int) {} };

// We also use a specialization for float.
template <> struct TplIndirectStruct2<float> { TplIndirectStruct2(float) {} };
