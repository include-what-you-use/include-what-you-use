//===--- typedef_chain_in_template-d3.h - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Container that tests typedef chains shorter than in libc++.

template<typename T>
class ContainerShortTypedefChain {
 private:
  T content_;
 public:
  T& GetContent1() { return content_; }

  typedef T& reference;
  reference GetContent2() { return content_; }
};
