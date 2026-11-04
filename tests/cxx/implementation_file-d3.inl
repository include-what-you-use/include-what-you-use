//===--- implementation_file-d3.inl - test input file for iwyu ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Made private to implementation_file-d3-all.h by implementation_file.imp.

inline int ImplFileD3Class::InlineMethod() {
  return 3;
}
