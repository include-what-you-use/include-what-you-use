//===--- fooa.h - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <memory>
class fooB;

std::unique_ptr<fooB> getPointerFooB() {
  return std::make_unique<fooB>();
}
