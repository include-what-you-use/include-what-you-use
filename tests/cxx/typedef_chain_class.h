//===--- typedef_chain_class.h - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Simple class to use in typedef chains.  Similar to IndirectClass but without
// concerns if it is a direct include or indirect.

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_TYPEDEF_CHAIN_CLASS_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_TYPEDEF_CHAIN_CLASS_H_

class TypedefChainClass {
 public:
  void Method() const { };
};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_TYPEDEF_CHAIN_CLASS_H_
