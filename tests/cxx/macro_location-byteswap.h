//===--- macro_location-byteswap.h - test input file for iwyu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#define bswap(x) ({ int __x = (x); bswap2(__x); })
#define bswap2(x) (x)
