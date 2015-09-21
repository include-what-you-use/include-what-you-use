//===--- typedef_chain_in_template-i1.h - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This header mimics ext/alloc_traits.h in libstdc++.

template<typename T>
struct TypedefWrapper {
  typedef T value_type;
  typedef value_type& reference;
};
