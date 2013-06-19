//===--- badinc-i5.h - test input file for iwyu ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This file defines some crazy macros, but is not actually
// included in real life (it's protected by '#if 0').  We want
// to make sure we don't take these macros seriously.

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_I5_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_I5_H_

#define hash_map map
#define printf   fabs

#endif // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_I5_H_
