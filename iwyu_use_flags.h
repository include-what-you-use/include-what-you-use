//===--- iwyu_use_flags.h - describe various contextual features of uses --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_IWYU_USE_FLAGS_H_
#define INCLUDE_WHAT_YOU_USE_IWYU_USE_FLAGS_H_

namespace include_what_you_use {

// Flags describing special features of a use that influence IWYU analysis.
typedef unsigned UseFlags;

// Normal use.
const UseFlags UF_None = 0;
// Use is inside a C++ method body.
const UseFlags UF_InCxxMethodBody = 1;
// Use is due to some redeclaration.
const UseFlags UF_RedeclUse = 2;
// Use targets an explicit instantiation.
const UseFlags UF_ExplicitInstantiation = 4;
}

#endif
