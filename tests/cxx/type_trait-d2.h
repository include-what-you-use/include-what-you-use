//===--- type_trait-d2.h - test input file for iwyu -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

class Derived;
class Class;
union Union1;

using DerivedPtrRefNonProviding = Derived*&;
using DerivedRefNonProviding = Derived&;
using ClassRefNonProviding = Class&;
using Union1RefNonProviding = Union1&;
