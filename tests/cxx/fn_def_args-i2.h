//===--- fn_def_args-i2.h - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

int GetInt();

void FnWithSmearedDefArgs(int, int = 0);
