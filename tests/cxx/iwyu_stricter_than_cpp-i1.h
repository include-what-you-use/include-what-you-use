//===--- iwyu_stricter_than_cpp-i1.h - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

struct IndirectStruct1 {
  IndirectStruct1(int) {
  }
  char c;
};
struct IndirectStructForwardDeclaredInD1 {
  IndirectStructForwardDeclaredInD1(int) {}
};

template <typename T> struct TplIndirectStruct1 { TplIndirectStruct1(int) {} };
template <typename T> struct TplIndirectStructForwardDeclaredInD1 {
  TplIndirectStructForwardDeclaredInD1(int) {}
};

// It is important to avoid any odr-use of NonInstantiated, NoAutocastForSpec,
// and AutocastInPartialSpec. They are introduced to test implicit ctor finding
// for "autocast" when class template specialization is not instantiated.

template <typename>
struct NonInstantiated {
  NonInstantiated(int) {
  }
};

template <typename>
struct NoAutocastForSpec {
  NoAutocastForSpec(int) {
  }
};
template <>
struct NoAutocastForSpec<char> {};

template <typename>
struct AutocastInPartialSpec {};
template <typename T>
struct AutocastInPartialSpec<T*> {
  AutocastInPartialSpec(int) {
  }
};

struct MappedToD1 {};
