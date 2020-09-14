//===--- unused_fwd_decl_simple.h - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

@class Unused;
@class Used;

void foo(Used *used);
