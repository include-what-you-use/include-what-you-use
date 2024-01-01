//===--- template_args-d2.h - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Aliases in this file provide neither TplInI1 nor IndirectClass declaration.

class IndirectClass;

template <typename>
struct TplInI1;

struct StructInD2 {
  static TplInI1<IndirectClass> t;
};

typedef decltype(StructInD2::t) NonProvidingTypedef;

namespace ns_in_d2 {
using ::NonProvidingTypedef;
}

template <int>
using NonProvidingAlias = ns_in_d2::NonProvidingTypedef;

using NonProvidingFunctionAlias1 = int(IndirectClass&);
using NonProvidingFunctionAlias2 = IndirectClass(int);
