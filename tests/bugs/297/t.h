//===--- t.h - iwyu test --------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <memory>

template <typename T>
class Template {
 public:
  typedef T Type;
  typedef std::shared_ptr<Type> TypeSPtr;

  void foo() {
    TypeSPtr obj = std::make_shared<Type>();
  }
};
