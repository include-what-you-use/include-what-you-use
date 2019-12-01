//===--- explicit_instantiation2-template_helpers.h - test input ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_EXPLICIT_INSTANTIATION2_TEMPLATE_HELPERS_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_EXPLICIT_INSTANTIATION2_TEMPLATE_HELPERS_H_

template <class T>
class Template;

template <class T>
class FullUseArg {
  T t;
};
template <class T>
class FwdDeclUseArg {
  T* t;
};
template <class U = short, class T = Template<U>>
class TemplateAsDefaultFull {
  T t;
};
template <class U = short, class T = Template<U>>
class TemplateAsDefaultFwd {
  T* t;
};
template <template <class> class T>
class TemplateTemplateArgShortFull {
  T<short> t;
};
template <template <class> class T>
class TemplateTemplateArgShortFwd {
  T<short>* t;
};

template <class T>
class ProvidedTemplate {};

template <class U = short, class T = ProvidedTemplate<U>>
class TemplateAsDefaultFullProvided {
  T t;
};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_EXPLICIT_INSTANTIATION2_TEMPLATE_HELPERS_H_
