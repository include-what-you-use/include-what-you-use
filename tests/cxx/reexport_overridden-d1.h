//===--- reexport_overridden-d1.h - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/indirect.h"

// From override_reexport-indirect.h
class FwdRetType;

class Base {
 public:
  virtual IndirectClass GetIndirect();
  virtual FwdRetType ReturnType();
  virtual FwdRetType ReturnTypeUnusedInBody();
  virtual void ArgumentUnused(const IndirectClass& x);
  virtual void ArgumentUnusedInline(const IndirectClass&);
};
