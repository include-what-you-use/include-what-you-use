//===--- default_template_arg_other_file-i2.h - test input file for iwyu --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

class MyClass {};

template<class T> class OperateOn;   // forward declare
template<> class OperateOn<MyClass> { };
