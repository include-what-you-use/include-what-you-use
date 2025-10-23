//===--- default_tpl_arg-i3.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This header should not be suggested for inclusion into .cc-file so as to be
// able to test that correct form of template forward-declaration is suggested.

template <typename>
class ClassTplWithDefinition4;

template <typename = int>
class ClassTplWithDefinition4 {};

template <typename = int>
class ClassTplWithDefinition5;

template <typename>
class ClassTplWithDefinition5 {};

template <typename = int>
class ClassTplWithDefinition6 {};

template <typename>
class ClassTplWithDefinition7;
