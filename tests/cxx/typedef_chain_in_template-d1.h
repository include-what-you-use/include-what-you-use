//===--- typedef_chain_in_template-d1.h - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This header mimics std::vector in libstdc++.

#include "tests/cxx/typedef_chain_class.h"
#include "tests/cxx/typedef_chain_in_template-i1.h"

template<typename T>
class ContainerAsLibstdcpp {
 public:
  typedef T value_type;
  typedef typename TypedefWrapper<T>::reference reference;
  reference GetContent() { return content_; }
 private:
  value_type content_;
};
