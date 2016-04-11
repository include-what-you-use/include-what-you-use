//===--- macro_defined_by_includer-g2.h - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef DIRECT_INCLUDE_GUARD_2
  #error Do not include directly
#else
  class GuardedInclude2 {};
#endif
