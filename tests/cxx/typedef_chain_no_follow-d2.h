//===--- typedef_chain_no_follow-d2.h - test input file for iwyu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Declares typedef inside a class.

#include "tests/cxx/typedef_chain_class.h"

class NonContainer1 {
 public:
  typedef TypedefChainClass value_type;
  typedef value_type& reference;

  value_type tcc_;
  reference GetTypedefChainClass() { return tcc_; }
};
