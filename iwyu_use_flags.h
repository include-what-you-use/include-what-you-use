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

const UseFlags UF_None = 0;
const UseFlags UF_InCxxMethodBody = 1;       // use is inside a C++ method body
const UseFlags UF_RedeclUse = 2;             // use is due to some redeclaration
const UseFlags UF_ExplicitInstantiation = 4; // use targets an explicit instantiation
}

#endif
