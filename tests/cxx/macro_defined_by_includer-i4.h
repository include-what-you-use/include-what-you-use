//===--- macro_defined_by_includer-i4.h - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifdef ENABLE_DEBUG
void foo() {
  // Extra code for debugging purposes.
}
#else
void foo() {}
#endif
