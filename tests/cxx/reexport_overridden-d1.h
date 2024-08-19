//===--- reexport_overridden-d1.h - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/reexport_overridden-i1.h"

class Derived1 : public Base {
 public:
  // Return type fully defined in Base.
  IndirectClass GetIndirect() override;

  // Return types forward-declared in Base.
  FwdRetType ReturnType() override;

  // Parameter with implicit conversion forward-declared in Base.
  void TakeFwdAutoconvParam(FwdAutoconvParam) override;
};

class Derived2 : public Derived1 {
 public:
  IndirectClass GetIndirect() override;
  FwdRetType ReturnType() override;
  void TakeFwdAutoconvParam(FwdAutoconvParam) override;
};
