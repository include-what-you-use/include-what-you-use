//===--- fn.h - iwyu test -------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

class Class;

template <typename>
class Tpl;

Class& GetClass();
Tpl<int>& GetTpl();
