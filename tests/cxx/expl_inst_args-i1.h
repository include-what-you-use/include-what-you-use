//===--- expl_inst_args-i1.h - iwyu test ----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

template <typename>
struct S2;

struct Struct1 {};
struct Struct2 {};
struct Struct3 {};

template <typename T = Struct1>
void body5() {
  T x;
}

using Struct3Providing = Struct3;
using Struct3ProvidingPtr = Struct3*;
using S2Struct3Providing = S2<Struct3>;
