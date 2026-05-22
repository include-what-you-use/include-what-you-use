//===--- expl_inst_args-i2.h - iwyu test ----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

struct Struct2;
template <typename>
struct S2;

using Struct2NonProviding = Struct2;
using Struct2NonProvidingPtr = Struct2*;
using S2Struct2NonProviding = S2<Struct2>;

template <typename T = Struct2>
void body4() {
  T x;
}
