//===--- type_trait-d2.h - test input file for iwyu -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

class Base;
class Derived;
class Class;
union Union1;

using ClassNonProviding = Class;
using DerivedPtrRefNonProviding = Derived*&;
using DerivedRefNonProviding = Derived&;
using ClassRefNonProviding = Class&;
using ClassConstRefNonProviding = const Class&;
using Union1RefNonProviding = Union1&;
using Union1PtrRefNonProviding = Union1*&;

template <typename T>
using BaseMemPtr = T Base::*;
template <typename T>
using DerivedMemPtr = T Derived::*;
template <typename T>
using UnionMemPtr = T Union1::*;
