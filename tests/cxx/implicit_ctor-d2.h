//===--- implicit_ctor-d2.h - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

struct NoAutocastCtor;
struct NoTrivialCtorDtor;

using NonProviding = NoAutocastCtor;
using NoTrivialCtorDtorNonProvidingAlias = NoTrivialCtorDtor;
