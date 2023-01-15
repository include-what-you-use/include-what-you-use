//===--- template_args-d1.h - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Aliases in this file provide TplInI1 declaration because it is directly
// included, but don't provide IndirectClass.

#include "tests/cxx/template_args-i1.h"

class IndirectClass;

struct StructInD1 {
  static TplInI1<IndirectClass> t;
};

typedef decltype(StructInD1::t) ProvidingTypedef;

namespace ns_in_d1 {
using ::ProvidingTypedef;
}

template <int>
using ProvidingAlias = ns_in_d1::ProvidingTypedef;
