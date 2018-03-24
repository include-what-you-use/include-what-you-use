//===--- range_for-iterable.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/indirect.h"

class Iterable {
 public:
  Iterable() : items(nullptr), count(0) {}

  const IndirectClass* begin() const { return items; }
  const IndirectClass* end() const { return items + count; }

 private:
  IndirectClass* items;
  unsigned count;
};
