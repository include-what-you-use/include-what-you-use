//===--- default_tpl_arg-i2.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

template <typename, typename = int>
class ClassTpl2;

template <typename>
class ClassTplWithDefinition2 {};

template <typename>
class ClassTplWithDefinition3 {};

template <typename = int>
class ClassTplWithDefinition7 {};

template <typename, typename = int>
using AliasTpl7 = int;
