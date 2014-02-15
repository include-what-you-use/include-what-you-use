//===--- include_with_using-d4.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Also test using a var, not a type;

namespace ns4 {
int var_in_d4 = 0;
}

using ns4::var_in_d4;
