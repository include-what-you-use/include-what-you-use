//===--- out_of_line-dtor.h - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "out_of_line-dtor-class.h"

inline ClassWithOutOfLineDtor::~ClassWithOutOfLineDtor() {
}
