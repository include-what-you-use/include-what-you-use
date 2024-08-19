//===--- reexport_overridden-i1.h - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/indirect.h"

// From reexport_overridden-i2.h
class FwdAutoconvParam;
class FwdRetType;

class Base {
 public:
  virtual IndirectClass GetIndirect();
  virtual FwdRetType ReturnType();
  virtual void TakeFwdAutoconvParam(FwdAutoconvParam);
};
