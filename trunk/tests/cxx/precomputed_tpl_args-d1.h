//===--- precomputed_tpl_args-d1.h - test input file for iwyu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests the precomputed template-arg-use list in iwyu_cache.cc.

#include "tests/cxx/precomputed_tpl_args-i1.h"

// Provide another class that we have a std::allocator specialization for.
class D1SpecializationClass { };

namespace std {
template<> class less<D1SpecializationClass> {
 public:
  bool operator()(const D1SpecializationClass&, const D1SpecializationClass&) {
    return true;
  }
};
}
