//===--- badinc-i2-inl.h - test input file for iwyu -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This defines a variety of methods, functions, classes, and
// variables that are declared in badinc-i2.h.  The goal is to make
// sure that badinc.cc realizes it needs these definitios, and not
// just the declarations in badinc-i2.h.

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_I2_INL_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_I2_INL_H_

#include "tests/badinc-i2.h"


inline I2_Class::I2_Class(const char*) {
}

inline I2_Class::~I2_Class() {
}

inline int I2_Class::InlFileFn() {
  return 3;
}

template<typename T> inline int I2_Class::InlFileTemplateFn() {
  return 4;
}

/*static*/ inline int I2_Class::InlFileStaticFn() {
  return 5;
}

template<typename T> inline I2_TemplateClass<T>::I2_TemplateClass(
    T a, const char* b) {
}

template<typename T> I2_TemplateClass<T>::~I2_TemplateClass() {
}

template<typename T> inline int I2_TemplateClass<T>::InlFileTemplateClassFn() {
  return 6;
}

class I2_InlFileClass {
 public:
  int a;
};

template<typename T> class I2_InlFileTemplateClass {
 public:
  T a;
};

inline int InlFileFreeFn() {
  return 7;
}

template<typename T> int InlFileFreeTemplateFn() {
  return 8;
}

template<> int InlFileFreeTemplateFn<int>() {
  return 9;
}

int inlfile_var;

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_I2_INL_H_
