//===--- expl_inst_select-i2.h - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Explicit instantiation declaration whose definition is in the main file
// (this declaration is unused).
extern template class Template<char>;

// Explicit instantiation declaration with a definition in -i3.h
// (this declaration should be preferred).
extern template class Template<short>;
