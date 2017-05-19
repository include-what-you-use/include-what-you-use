//===--- hash_error-nomap.h - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifdef IWYU_THIS_SHOULD_NEVER_BE_DEFINED
#error "This is just some random #error"
#endif

int nomap_symbol;

