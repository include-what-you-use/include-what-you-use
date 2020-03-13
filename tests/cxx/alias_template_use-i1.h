//===--- alias_template_use-i1.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "alias_template_use-i2.h"

template<typename T>
using AliasTemplate = AliasedTemplate<T>;

