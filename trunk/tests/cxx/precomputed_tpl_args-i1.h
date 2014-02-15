//===--- precomputed_tpl_args-i1.h - test input file for iwyu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <functional>

class IndirectClass { };

// Provide another class that we have a std::allocator specialization for.
class SpecializationClass { };

namespace std {
template<> class less<SpecializationClass> {
 public:
  bool operator()(const SpecializationClass&, const SpecializationClass&) {
    return true;
  }
};
}
