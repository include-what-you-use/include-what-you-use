//===--- typedef_chain_in_template-d4.h - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Container that tests typedef chains longer than in libc++.

template<typename T>
class ContainerLongTypedefChain {
 public:
  typedef T value_type;
  typedef value_type& reference;
  typedef reference& reference_reference;
  reference_reference GetContent1() { return content_; }

  typedef reference reference2;
  typedef reference2 reference3;
  reference3 GetContent2() { return content_; }
 private:
  value_type content_;
};
