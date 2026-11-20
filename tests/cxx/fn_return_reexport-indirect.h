//===--- fn_return_reexport-indirect.h - test input file for iwyu ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

struct Indirect {
  // The destructor declaration makes this class non-trivial.
  ~Indirect();
  void Method() const;
  void OverloadedMethod(int) const;
  void OverloadedMethod(int*) const;
  int* begin() const;
  int* end() const;
  operator bool() const;
  Indirect& operator-() const;
};
