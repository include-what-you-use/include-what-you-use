//===--- funcs.h - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <utility>
#include <vector>

std::pair<int, int> GetPair();
std::vector<std::pair<int, int>> GetVectorOfPairs();
