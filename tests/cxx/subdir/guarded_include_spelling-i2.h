//===--- guarded_include_spelling-i2.h - test input file for iwyu ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef SPELLING_I1_INCLUDED
#error "Do not #include this file directly: #include -i1.h."
#endif

struct SpellingI2 {};
