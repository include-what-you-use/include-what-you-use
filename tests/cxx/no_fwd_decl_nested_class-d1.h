//===--- no_fwd_decl_nested_class-d1.h - test input file for iwyu ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

struct DirectOuterClass {
  class NestedClass;
};
