//===--- explicit_instantiation2-template_short.h - test input ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_EXPLICIT_INSTANTIATION2_TEMPLATE_SHORT_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_EXPLICIT_INSTANTIATION2_TEMPLATE_SHORT_H_

extern template class Template<short>;

extern template class ProvidedTemplate<short>;

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_EXPLICIT_INSTANTIATION2_TEMPLATE_SHORT_H_
