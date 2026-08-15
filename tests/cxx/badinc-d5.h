//===--- badinc-d5.h - test input file for iwyu ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_BADINC_D5_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_BADINC_D5_H_

#include "tests/cxx/badinc-d1.h"
#include "tests/cxx/badinc-d2.h"
#include <string>

// Some declarations from badinc.cc have been duplicated here to test them with
// a different provision policy (only header declarations can provide).

// The generic D5_OperateOn, but each specialization needs to define its own.
template <class T>
class D5_OperateOn {};

// D5_OperateOn isn't checked for IWYU violations until it's instantiated.
template <class T, class Functor = D5_OperateOn<T> >
class D5_TemplateStructHelper {
 public:
  void a() {
    Functor f;
    (void)f;
  }
};

// To make this example as much like hash_set<> as possible, the outer
// class is really just a container around the class that does work.
template <class T, class Functor = D5_OperateOn<T> >
class D5_TemplateStruct {
 private:
  typedef D5_TemplateStructHelper<T, Functor> _TS;
  _TS ts;

 public:
  void a() {
    return ts.a();
  }
};

typedef std::string D5_string;  // Nobody should use this.
// IWYU: I1_Class is...*badinc-i1.h
typedef I1_Class D5_typedef;
// IWYU: kI1ConstInt is...*badinc-i1.h
// IWYU: I1_Class is...*badinc-i1.h
typedef I1_Class D5_typedef_array[kI1ConstInt];
// We need the full definition of template types (I1_TemplateClass in
// this case) since we're re-exporting them.  Note we need a full
// definition even of I2_Class, since we don't know if clients will be
// using the no-arg Cc_tpl_typedef ctor, which requires the full
// definition of I2_Class.
// IWYU: I1_TemplateClass is...*badinc-i1.h...*#included.
// IWYU: I1_Class is...*badinc-i1.h
// IWYU: I2_Class is...*badinc-i2.h
// IWYU: I2_Class::~I2_Class() is...*badinc-i2-inl.h
typedef I1_TemplateClass<I1_TemplateClass<I1_Class, I2_Class> > D5_tpl_typedef;
inline void Instantiate_I1_TemplateClass() {
  // TODO(csilvers): it would be nice to be able to take this line out and
  // still have the above tests pass:
  // TODO(bolshakov): figure out how to determine at the use site that a typedef
  // provides not only the types but also the member functions.
  // IWYU: I2_Class::~I2_Class() is...*badinc-i2-inl.h
  D5_tpl_typedef d5_tpl_typedef;
}
// IWYU: I2_Class is...*badinc-i2.h
// IWYU: I2_Class::I2_Class({{.*}}) is...*badinc-i2-inl.h
// IWYU: I2_Class::~I2_Class() is...*badinc-i2-inl.h
// IWYU: I2_Class::InlFileFn({{.*}}) is...*badinc-i2-inl.h
// IWYU: I2_Class::InlFileTemplateFn({{.*}}) is...*badinc-i2-inl.h
// IWYU: I2_Class::InlFileStaticFn({{.*}}) is...*badinc-i2-inl.h
typedef I2_Class D5_I2_Class_Typedef;
// I1_Struct isn't really used by any possible operation with H_TemplateStruct,
// but '#include' is required as a common rule, as long as I1_Struct
// isn't forward-declared.
// IWYU: I1_Struct is...*badinc-i1.h
// IWYU: D5_OperateOn<I1_Struct> is...*badinc-i1.h
typedef D5_TemplateStruct<I1_Struct> D5_TemplateStruct_I1Struct_Typedef;
inline void Instantiate_D5_TemplateStruct() {
  D5_TemplateStruct_I1Struct_Typedef x;
  // IWYU: D5_OperateOn<I1_Struct> is...*badinc-i1.h
  x.a();
}

// IWYU: I2_TemplateClass needs a declaration
// IWYU: I2_TemplateClass is...*badinc-i2.h
// IWYU: I2_TemplateClass::I2_TemplateClass<{{.*}}>({{.*}}) is...*badinc-i2-inl.h
// IWYU: I2_TemplateClass::~I2_TemplateClass<{{.*}}>() is...*badinc-i2-inl.h
// IWYU: I2_TemplateClass::InlFileTemplateClassFn({{.*}}) is...*badinc-i2-inl.h
typedef I2_TemplateClass<int> D5_typedef_implicit_instantiation;
// Make sure we can do the same typedef multiple times.
// IWYU: I2_TemplateClass needs a declaration
// IWYU: I2_TemplateClass is...*badinc-i2.h
// IWYU: I2_TemplateClass::I2_TemplateClass<{{.*}}>({{.*}}) is...*badinc-i2-inl.h
// IWYU: I2_TemplateClass::~I2_TemplateClass<{{.*}}>() is...*badinc-i2-inl.h
// IWYU: I2_TemplateClass::InlFileTemplateClassFn({{.*}}) is...*badinc-i2-inl.h
typedef I2_TemplateClass<int> D5_typedef_implicit_instantiation2;

// TODO(csilvers): I1_Class is technically forward-declarable.
// IWYU: I1_Class is...*badinc-i1.h
// IWYU: I1_Enum is...*badinc-i1.h
template <class T = I1_Class, I1_Enum E = I11>
class D5_DeclareOnlyTemplateClass;

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_BADINC_D5_H_

/**** IWYU_SUMMARY

tests/cxx/badinc-d5.h should add these lines:
#include "tests/cxx/badinc-i1.h"
#include "tests/cxx/badinc-i2-inl.h"
#include "tests/cxx/badinc-i2.h"

tests/cxx/badinc-d5.h should remove these lines:
- #include "tests/cxx/badinc-d1.h"  // lines XX-XX
- #include "tests/cxx/badinc-d2.h"  // lines XX-XX
- template <class T = I1_Class, I1_Enum E = I11> class D5_DeclareOnlyTemplateClass;  // lines XX-XX+1

The full include-list for tests/cxx/badinc-d5.h:
#include <string>  // for string
#include "tests/cxx/badinc-i1.h"  // for D5_OperateOn, I1_Class, I1_Enum, I1_Struct, I1_TemplateClass, kI1ConstInt
#include "tests/cxx/badinc-i2-inl.h"  // for I2_Class::I2_Class, I2_Class::InlFileFn, I2_Class::InlFileStaticFn, I2_Class::InlFileTemplateFn, I2_Class::~I2_Class, I2_TemplateClass::I2_TemplateClass<FOO>, I2_TemplateClass::InlFileTemplateClassFn, I2_TemplateClass::~I2_TemplateClass<FOO>
#include "tests/cxx/badinc-i2.h"  // for I2_Class, I2_TemplateClass

***** IWYU_SUMMARY */
