//===--- overloaded_operator-indirect.h - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

struct InnerOperatorStruct {
  bool operator==(const InnerOperatorStruct&) const;
};

struct Derived : InnerOperatorStruct {};
