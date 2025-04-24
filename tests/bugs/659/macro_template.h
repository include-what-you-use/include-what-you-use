//===--- macro_template.h - iwyu test -------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#define MACRO() MacroClass()

class MacroClass {
public:
  template <typename T> T func() { return T(); }
};
