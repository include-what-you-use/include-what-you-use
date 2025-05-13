//===--- bar-fwd.h - iwyu test --------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef BAR_FWD_H
#define BAR_FWD_H

template <typename T = int>
class Bar;

#endif  // BAR_FWD_H
