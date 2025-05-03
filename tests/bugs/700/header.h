//===--- header.h - iwyu test ---------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

namespace FooNs {
  class FooClass {};
};

namespace BarNs {
  using namespace FooNs;

  class BarClass {
  };
}
