//===--- iwyu_stricter_than_cpp-i1.h - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

struct IndirectStruct1 { IndirectStruct1(int) {} };
struct IndirectStructForwardDeclaredInD1 {
  IndirectStructForwardDeclaredInD1(int) {}
};

template <typename T> struct TplIndirectStruct1 { TplIndirectStruct1(int) {} };
template <typename T> struct TplIndirectStructForwardDeclaredInD1 {
  TplIndirectStructForwardDeclaredInD1(int) {}
};
