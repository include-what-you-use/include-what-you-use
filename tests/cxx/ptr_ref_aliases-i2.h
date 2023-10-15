//===--- ptr_ref_aliases-i2.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef PTR_REF_ALIASES_I2_H_
#define PTR_REF_ALIASES_I2_H_

#include "tests/cxx/ptr_ref_aliases-d3.h"

struct Indirect : IndirectBase {
  void Method();
  int* begin() const;
  int* end() const;
  Indirect& operator<<(int);
};

#endif
