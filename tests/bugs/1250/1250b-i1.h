//===--- 1250b-i1.h - iwyu test ------* c++ *------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// A template with methods and an iterator type.
// Inspired by Clang's AST type Redeclarable, which shows this problem in
// iwyu_ast_util.cc.

template<class T>
class Template {
public:
  using iterator = T*;

  Template<T>::iterator begin();
  Template<T>::iterator end();
};
