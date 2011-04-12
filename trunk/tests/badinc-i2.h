//===--- badinc-i2.h - test input file for iwyu ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_I2_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_I2_H_

#include <vector>
#include <list>    // I only ever use a pointer to stuff in this file
#include <set>     // only used in badinc.h

#define I2_MACRO  2

// The types.
enum I2_Enum { I21, I22, I23, I2_LAST = I23 };

enum I2_EnumForTypedefs { I21t, I22t, I23t, I2t_LAST = I23t };

typedef I2_EnumForTypedefs I2_Typedef;

struct I2_Struct {
  int a;
  float b;
  I2_Enum c;
  operator bool() { return true; }
};

union I2_Union {
  I2_Struct a;
  I2_Enum b;
  int c;
};

class I2_Class {
 public:
  I2_Class(int a) { a_ = a; }
  I2_Class(I2_Struct& s) { }
  I2_Class(I2_Union u) { }
  I2_Class() { a_ = 1; }
  operator int() { return a_; }
  operator I2_Union() { I2_Union retval; return retval; }
  inline I2_Class(const char*);    // defined in badinc-i2-inl.h
  inline ~I2_Class();              // defined in badinc-i2-inl.h
  int a() { return a_; }
  int size() { return a_; }        // needed by I1_TemplateMethodOnlyClass::e()
  I2_Typedef b() const { return static_cast<I2_EnumForTypedefs>(a_); }
  int CcFileFn();                              // defined in badinc.cc
  int AnotherTranslationUnitFn();              // defined in badinc-extradef.cc
  inline int InlFileFn();                      // defined in badinc-i2-inl.h
  template<typename T> inline int InlFileTemplateFn();  // ditto
  static inline int InlFileStaticFn();         // ditto
  static int s;                                // defined in badinc-extradef.cc
 private:
  int a_;
};

template<typename FOO>
class I2_TemplateClass {
 public:
  I2_TemplateClass(FOO a) { a_ = a; }
  inline I2_TemplateClass(FOO a, const char* b); // defined in badinc-i2-inl.h
  ~I2_TemplateClass();                           // defined in badinc-i2-inl.h
  FOO a() { return a_; }
  int CcFileFn();                                // defined in badinc.cc
  inline int InlFileTemplateClassFn();           // defined in badinc-i2-inl.h
 private:
  FOO a_;
};

template<typename FOO> class I2_TypedefOnly_Class {
 public:
  typedef FOO value_type;
  template<typename BAR> void TplFunction(BAR* bar) { }
};

// FOO should be an I1_Class* or I2_Class*.
template<typename FOO> int I2_TemplateFn(int i, FOO f) {
  return i + f->a();
}

typedef I2_Class* I2_StructPtr;

typedef int (*I2_FunctionPtr)(int, I2_Enum);

inline I2_Struct I2_Function(I2_Class* c) {
  return I2_Struct();
}

inline void I2_UnionFunction(const I2_Union& u) { }

inline void I1_And_I2_OverloadedFunction(float f) { }

template <typename A> struct TemplateForHClassTplFn {
  A value;
};

class I2_OperatorDefinedInI1Class {
 public:
  I2_OperatorDefinedInI1Class& operator<<(int i);
};

class I2_ForwardDeclareClass;

class I2_ThisClassIsOnlyNewed {
};
class I2_ThisClassIsOnlyDeleted {
};
class I2_ThisClassIsOnlyDeletedPtr {
};
class I2_ThisClassIsOnlySubclassed {
 public:
  int a() { return 5; }
};
class I2_ThisClassIsOnlySubclassedWithVirtualMethod {
 public:
  virtual int Impl() { return 1; }
  virtual int Abstract() = 0;
  virtual int BaseOnly() { return 2; }
  int NonvirtualBaseOnly() { return 3; }
  virtual ~I2_ThisClassIsOnlySubclassedWithVirtualMethod() { }
};
class I2_Subclass : public I2_ThisClassIsOnlySubclassedWithVirtualMethod {
 public:
  virtual int Impl() { return 2; }
  virtual int Abstract() { return 3; }
  virtual ~I2_Subclass() { }
};

class I2_InlFileClass;   // defined in badinc-i2-inl.h
template<typename T> class I2_InlFileTemplateClass;  // ditto

inline int InlFileFreeFn();   // defined in badinc-d1-inl.h
template<typename T> int InlFileFreeTemplateFn();   // ditto

extern int inlfile_var;   // defined in badinc-d1-inl.h

#if 1
  # define MACRO_CALLING_I2_FUNCTION  I2_Function(NULL)
#endif

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_I2_H_
