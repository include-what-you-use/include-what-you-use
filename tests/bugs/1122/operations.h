//===--- operations.h - iwyu test -----------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef OPERATIONS_H
#define OPERATIONS_H

#define MULTIPLY_OP() \
int operator *(MyInt lhs, int rhs) { \
  return lhs.value * rhs; \
}

#endif  // OPERATIONS_H
