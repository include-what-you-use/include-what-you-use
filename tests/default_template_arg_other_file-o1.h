//===--- default_template_arg_other_file-o1.h - test input file for iwyu --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// The generic OperateOn, but each specialization needs to define its own.
template<class T> class OperateOn { };
