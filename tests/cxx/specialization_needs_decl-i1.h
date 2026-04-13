//===--- specialization_needs_decl-i1.h - test input file -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This is employed to show that when the template specialization is used, the
// base template is not required in full. Issue #735.

// Base template
template<typename>
struct Template;

// Specialization for int
template<> struct Template<int> { int x; };

template <typename>
void TplFn1() {
}

template <typename>
void TplFn2() {
}

template <typename>
void TplFn3() {
}

template <typename>
int var_tpl1;

template <typename>
int var_tpl2;

template <typename>
int var_tpl3;
