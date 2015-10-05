//===--- typedef_chain_in_template-d2.h - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This header mimics std::vector in libc++.

template<typename T>
class ContainerAsLibcpp {
 public:
  typedef T value_type;
  typedef value_type& reference;
  reference GetContent() { return content_; }
 private:
  value_type content_;
};
