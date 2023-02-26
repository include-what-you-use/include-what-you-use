//===--- expl_inst_select-i3.h - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Explicit instantiation definition used in main file.
template class Template<double>;

// Explicit instantiation definition with declaration in -i2.h (which is
// preferred to this definition).
template class Template<short>;
