//===--- badinc.cc - test input file for iwyu -----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This is a unittest for include-what-you-use.
//
// NOTE: Historically, all end-to-end testing was done in this file,
// NOTE: which is why it is so large and covers so many things.
// NOTE: However, new tests should *NOT* be added to this file.
// NOTE: Instead, create a new .cc file in this directory (tests/cxx/)
// NOTE: that tests just one aspect of include-what-you-use.
//
// This unittest tries to test most combination of good and bad
// possibilities: functions that are and aren't declared in the
// right place, vars that are and aren't declared in the right
// place, var types that are and aren't declared in the right place.
// We get the list of types-of-interest from
//   https://developer.mozilla.org/En/Dehydra/Object_Reference#Type_Objects
//
// There are several files (this is not necesarily a complete list):
//    badinc.cc: this file, part of the 'compilation unit'.
//    badinc.h: also part of the 'compilation unit'.
//    badinc-inl.h: also part of the 'compilation unit'.
//    badinc-d1.h: directly included from badinc.cc
//    badinc-d2.h: directly included from badinc.h and badinc.cc (ptr only use)
//    badinc-d3.h: included from badinc_d1.h and badinc.h: a direct include
//    badinc-d4.h: included from badinc.h: tests operators and ptr-only
//    badinc-i1.h: included from badinc_d1.h -- an indirect include
//    badinc-i2.h: included from badinc_d2.h and from badinc_i1.h
//    badinc-i3.h: included from badinc_d3.h (forward declarations only).
//    badinc-i4.h: included from badinc_d1.h (macro definitions only).
//    badinc-i5.h: included from badinc_i1.h (unused macro definitions only).
//    badinc-i6.h: included from badinc_i1.h (more macro definitions).
//
// Every IWYU violation is commented, in the line above, by what the
// violation is:
//   // IWYU: <symbol or type name> is...*<file where type is defined>
// The '...*' is just a normal '.*'.  The extra dots are added to improve
// readability.

#define USED_INC    "tests/cxx/badinc-d4.h"
#define UNUSED_INC  <locale>

#if 0
// computed #includes.
#include <locale>
#include "tests/cxx/badinc-d4.h"
#endif

// Some of the #include lines are deliberately formatted wrong, in
// order to test that IWYU handles them correctly.
#include  <math.h>    // not actually used
#include <typeinfo>   // for typeid
#include "tests/cxx/badinc-inl.h"
#include  "tests/cxx/badinc.h"
#include   "tests/cxx/badinc-d1.h"
#include "tests/cxx/badinc-d2.h"
#include  USED_INC    // testing computed #includes
#include  <string>    // not sorted properly (before the #include ""'s).
// The following line is needed, but use a 'keep' pragma anyway.
#include <fstream>    // IWYU pragma: keep
#include <algorithm>  // for find
#include UNUSED_INC
// The following ilne is not needed, but use a 'keep' pragma anyway.
#include <setjmp.h>   // IWYU pragma: keep
#include <cwchar>     // for NULL (though we get NULL via badinc.h's stdio.h).
#include <algorithm>  // try #including the same file twice
#include <algorithm>  // ...and then 3 times

#define CONCAT(a, b)  a##b

// This should given an IWYU error even though MACRO_CALLING_I1_MACRO
// is never actually called.
// IWYU: MACRO_CALLING_I6_FUNCTION is...*badinc-i1.h
#define MACRO_CALLING_I1_MACRO    MACRO_CALLING_I6_FUNCTION

// IWYU: I1_MACRO_SYMBOL_WITHOUT_VALUE is...*badinc-i1.h
#ifdef I1_MACRO_SYMBOL_WITHOUT_VALUE
int i1_macro_symbol_without_value_var;
#endif

// IWYU: I1_MACRO_SYMBOL_WITHOUT_VALUE is...*badinc-i1.h
#ifndef I1_MACRO_SYMBOL_WITHOUT_VALUE
int i1_macro_symbol_without_value_var;
#endif

// IWYU: I1_MACRO_SYMBOL_WITH_VALUE is...*badinc-i1.h
#if I1_MACRO_SYMBOL_WITH_VALUE
int i1_macro_symbol_with_value_var;
#endif

// IWYU: I1_MACRO_SYMBOL_WITH_VALUE0 is...*badinc-i1.h
#if I1_MACRO_SYMBOL_WITH_VALUE0
int 11_macro_symbol_with_value0_var;
// IWYU: I1_MACRO_SYMBOL_WITH_VALUE2 is...*badinc-i1.h
#elif I1_MACRO_SYMBOL_WITH_VALUE2
int i1_macro_symbol_with_value2_var;
#endif

// IWYU: I1_MACRO_SYMBOL_WITHOUT_VALUE is...*badinc-i1.h
#if defined(I1_MACRO_SYMBOL_WITHOUT_VALUE)
int i1_macro_symbol_without_value_var2;
#endif

#if 0
// IWYU: I1_MACRO_SYMBOL_WITHOUT_VALUE is...*badinc-i1.h
#elif defined(I1_MACRO_SYMBOL_WITHOUT_VALUE)
int i1_macro_symbol_without_value_var3;
#endif

// IWYU: I1_MACRO_SYMBOL_WITH_VALUE is...*badinc-i1.h
// IWYU: I1_MACRO_SYMBOL_WITH_VALUE2 is...*badinc-i1.h
#if I1_MACRO_SYMBOL_WITH_VALUE && I1_MACRO_SYMBOL_WITH_VALUE2
int i1_macro_symbol_with_value_and_value2_var;
#endif

// A slightly more complex example.
#if 0
// IWYU: I1_MACRO_SYMBOL_WITH_VALUE is...*badinc-i1.h
// IWYU: I1_MACRO_SYMBOL_WITH_VALUE2 is...*badinc-i1.h
#elif !I1_MACRO_SYMBOL_WITH_VALUE && !(I1_MACRO_SYMBOL_WITH_VALUE2)
int i1_macro_symbol_with_value_and_value2_var2;
#endif

// Test we don't try to report an iwyu violation for a built-in macro
#if __LINE__ || __STDC__ || defined(__cplusplus)
#endif

// Using declarations and statements.
// TODO(csilvers): I don't see a consistent way to say whether
// "i1_ns2" is an iwyu violation or not, since namespaces can be
// re-opened in many different files.  So let this go, even though all
// uses of this namespace are in badinc-i1.h; we'll get the iwyu
// violation later when we try to use symbols from i1_ns2.
using namespace i1_ns2;
// IWYU: i1_ns3::i1_int_global3 is...*badinc-i1.h
using i1_ns3::i1_int_global3;
// IWYU: i1_ns4 is...*badinc-i1.h
namespace cc_ns_alias = i1_ns4;
// IWYU: i1_ns::I1_NamespaceStruct needs a declaration
using i1_ns::I1_NamespaceStruct;
// IWYU: i1_ns::I1_NamespaceTemplateFn is...*badinc-i1.h
using i1_ns::I1_NamespaceTemplateFn;
// TODO(csilvers): mark this using declaration as redundant and remove it?
// IWYU: i1_ns::I1_UnusedNamespaceStruct needs a declaration
using i1_ns::I1_UnusedNamespaceStruct;
// TODO(csilvers): mark this using statement as redundant and remove it.
using namespace i1_ns5;

// We do some proper forward-declaring here, and also some unnecessary
// forward-declaring.  And of course we leave a lot of necessary
// forward-declaring out.
class I3_ForwardDeclareClass;
template<typename T>
struct I3_SimpleForwardDeclareTemplateStruct;
class I3_UnusedClass;
namespace i3_ns1 { namespace i3_ns2 { namespace i3_ns3 {
struct I3_ForwardDeclareNamespaceStruct;
} } }

class ForwardDeclareOnlyClass;
ForwardDeclareOnlyClass* forward_declare_only;

class MacroClass;

class ForwardDeclareOnlyForTypedefClass;
typedef ForwardDeclareOnlyForTypedefClass NeedFwdDeclarationForThisClass;

// Here's an example of specialization where we never define the base
// class.  IWYU should not ask us to remove the initial declaration!
template <class T> struct Cc_OnlySpecializedStruct;
template<> struct Cc_OnlySpecializedStruct<int> { };
Cc_OnlySpecializedStruct<int>* cc_onlyspecializedstruct_ptr;

// Try a type that only needs a forward-declare because of the language linkage.
extern "C" struct Cc_C_Struct;
struct Cc_C_Struct* cc_c_struct_ptr = NULL;

// The types.
typedef std::string Cc_string;   // Nobody should use this.
// IWYU: I1_Class is...*badinc-i1.h
typedef I1_Class Cc_typedef;
// IWYU: kI1ConstInt is...*badinc-i1.h
// IWYU: I1_Class is...*badinc-i1.h
typedef I1_Class Cc_typedef_array[kI1ConstInt];
// We need the full definition of template types (I1_TemplateClass in
// this case) since we're re-exporting them.  Note we need a full
// definition even of I2_Class, since we don't know if clients will be
// using the no-arg Cc_tpl_typedef ctor, which requires the full
// definition of I2_Class.
// IWYU: I1_Class needs a declaration
// IWYU: I2_Class needs a declaration
// IWYU: I1_TemplateClass is...*badinc-i1.h.*#included\.
// IWYU: I1_TemplateClass is...*badinc-i1.h.*for autocast
// IWYU: I1_Class is...*badinc-i1.h
// IWYU: I2_Class is...*badinc-i2.h
// IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
typedef I1_TemplateClass<I1_TemplateClass<I1_Class,I2_Class> > Cc_tpl_typedef;
// TODO(csilvers): it would be nice to be able to take this line out and
// still have the above tests pass:
Cc_tpl_typedef cc_tpl_typedef;
// IWYU: I2_Class is...*badinc-i2.h
// IWYU: I2_Class::I2_Class is...*badinc-i2-inl.h
// IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
// IWYU: I2_Class::InlFileFn is...*badinc-i2-inl.h
// IWYU: I2_Class::InlFileTemplateFn is...*badinc-i2-inl.h
// IWYU: I2_Class::InlFileStaticFn is...*badinc-i2-inl.h
typedef I2_Class Cc_I2_Class_Typedef;
// IWYU: I1_Struct needs a declaration
// IWYU: OperateOn is...*badinc-i1.h
typedef H_TemplateStruct<I1_Struct> Cc_H_TemplateStruct_I1Class_Typedef;

// IWYU: kI1ConstInt is...*badinc-i1.h
enum Cc_Enum { CC1 = kI1ConstInt };

struct Cc_Struct {
  // IWYU: kI1ConstInt is...*badinc-i1.h
  int a : kI1ConstInt;
  float b;
  // IWYU: I1_Enum is...*badinc-i1.h
  I1_Enum d;
};

// This is best instantiated with T=I1_Class. :-)
template<class T> class Cc_TypenameTemplateStruct {
  typename T::I1_Class_int a;
  struct T::NestedStruct c;
};

// Needs full type information for CC_Subclass's (implicit) dtor of scoped_ptr.
// IWYU: I2_Class is...*badinc-i2.h
// IWYU: I2_ThisClassIsOnlySubclassed is...*badinc-i2.h
// IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
class Cc_Subclass : public I2_ThisClassIsOnlySubclassed {
 public:
  // IWYU: I1_MemberPtr is...*badinc-i1.h
  Cc_Subclass() : ptr_(&I1_MemberPtr::a), scoped_ptr_() {
    // IWYU: I1_MemberPtr is...*badinc-i1.h
    I1_MemberPtr m;
    (m.*ptr_)();
    a();   // defined in the superclass
  }
  // IWYU: I1_MemberPtr is...*badinc-i1.h
  int (I1_MemberPtr::*ptr_)();
  // IWYU: I2_Class needs a declaration
  H_ScopedPtr<I2_Class> scoped_ptr_;

  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: I2_Class is...*badinc-i2.h
  int foo() throw(I1_Class, I2_Class);
};

// IWYU: I2_ThisClassIsOnlySubclassed is...*badinc-i2.h
class MultipleInheritanceSubclass : public I2_ThisClassIsOnlySubclassed,
                                    // IWYU: I2_Class is...*badinc-i2.h
                                    virtual I2_Class {
 public:
  // IWYU: I2_Class is...*badinc-i2.h
  // IWYU: I2_Struct is...*badinc-i2.h
  // IWYU: I2_MACRO is...*badinc-i2.h
  MultipleInheritanceSubclass() : I2_Class(I2_Struct().a + I2_MACRO) { }
  // IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
  ~MultipleInheritanceSubclass() { }
};

// This template struct is never instantiated, and neither is the
// struct that includes it as a member.  Even so, we should call its
// methods to be instantiated for our testing, so we notice the
// requirement for I1_Class's full type (via the default constructor).
template<class T> struct NeverCalledTemplateStruct {
  NeverCalledTemplateStruct() { T t; }
};
// IWYU: I1_Class is...*badinc-i1.h
struct NeverCalledStruct {
  // IWYU: I1_Class needs a declaration
  NeverCalledTemplateStruct<I1_Class> c;
};

struct Cc_DeclOrderStruct {
  void Fn() {
    // This should not require a fwd-decl even though it's not defined
    // until after its use: that's legal inside a class/struct.
    NestedStructDefinedAfterUse* n;
    (void)n;
  }
  struct NestedStructDefinedAfterUse {};
};

// IWYU: I1_Enum is...*badinc-i1.h
// IWYU: I11 is...*badinc-i1.h
template<class T, I1_Enum E = I11> struct Cc_TemplateStruct { };
// IWYU: I1_Enum is...*badinc-i1.h
// IWYU: I1_Class needs a declaration
// IWYU: I1_Class is...*badinc-i1.h
template<I1_Enum E> struct Cc_TemplateStruct<I1_Class, E> { I1_Class i; };
// IWYU: I1_Class needs a declaration
// IWYU: I12 is...*badinc-i1.h
template<> struct Cc_TemplateStruct<I1_Class, I12> { I1_Class* i; };
// TODO(csilvers): I1_Class is technically forward-declarable.
// IWYU: I1_Class is...*badinc-i1.h
// IWYU: I1_Enum is...*badinc-i1.h
// IWYU: I11 is...*badinc-i1.h
template<class T = I1_Class, I1_Enum E = I11> class Cc_DeclareOnlyTemplateClass;

// I2_Class has a non-explicit constructor (actually, two), so we need
// the full type here even though it's a const reference.  I1_Class
// has no implicit, one-argument constructor, so fwd declaring is ok.
// IWYU: I2_Class needs a declaration
const I2_Class& Cc_Function(
    // IWYU: I1_Class needs a declaration
    const I1_Class& i1,
    // IWYU: I2_Class is...*badinc-i2.h.*for autocast
    // IWYU: I2_Class needs a declaration
    const I2_Class& i2,
    // A subtle c++ point: forward-declaring is ok for i2b, because
    // you can't do implicit conversion to a non-const reference
    // (implicit conversion involves creating a temporary, which
    // doesn't bind to non-const references).
    // IWYU: I2_Class needs a declaration
    I2_Class& i2_nonconst,
    // Forward-declaring is ok because we a const reference to a *pointer*.
    // IWYU: I2_Class needs a declaration
    I2_Class* const & i2_ptrref,
    // IWYU: I1_Class is...*badinc-i1.h
    I1_Class i1_nonref,
    // IWYU: I2_Class is...*badinc-i2.h
    I2_Class i2_nonref) {
  // IWYU: I2_Class is...*badinc-i2.h
  // IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
  static I2_Class retval;   // something we can safely return a reference to
  return retval;
}

// Now try the same, but with templatized functions (which can't have
// default arguments, who knew?)
// IWYU: I1_Enum is...*badinc-i1.h
template<class T, I1_Enum E> int Cc_TemplateFunction() { return T().a(); }
// IWYU: I1_Class needs a declaration
// IWYU: I12 is...*badinc-i1.h
template<> int Cc_TemplateFunction<I1_Class, I12>() { return 3; }
// IWYU: I1_Class needs a declaration
// IWYU: I1_Class is...*badinc-i1.h
// IWYU: I13 is...*badinc-i1.h
template<> int Cc_TemplateFunction<I1_Class, I13>() { return I1_Class().a(); }
// IWYU: I1_Enum is...*badinc-i1.h
template<class T, I1_Enum E> T Cc_DeclareOnlyTemplateFunction();
// TODO(csilvers): also test calling these functions, including an
//                 implicit specialization.

template<class T> int Cc_TemplateNewer() { new T; return 0; }

// Here's a tricky example of template specialization taken from
// http://msdn.microsoft.com/en-us/library/3967w96f%28VS.80%29.aspx
template <class T> struct Cc_PTS {
  enum { IsPointer = 0, IsPointerToDataMember = 0 };
};
template <class T> struct Cc_PTS<T*> {
  enum { IsPointer = 1, IsPointerToDataMember = 0 };
};
template <class T, class U> struct Cc_PTS<T U::*> {
  enum { IsPointer = 0, IsPointerToDataMember = 1 };
};

template<class T,int> class Cc_TemplateSubclass : public H_TemplateTypedef { };

// Let's test that we detect overloaded functions correctly when all
// overloads are in the same file, and then when they're in different files.
template<typename T> void CallOverloadedFunctionSameFile(T t) {
  // IWYU: I1_OverloadedFunction is...*badinc-i1.h
  I1_OverloadedFunction(t);
}

template<typename T> void CallOverloadedFunctionDifferentFiles(T t) {
  // Since this method is overloaded in several places, we do the iwyu
  // check at the instantiation site, not here.
  I1_And_I2_OverloadedFunction(t);
}

template<typename T> void CallOverloadWithUsingShadowDecl(T t) {
  // This is only defined in one place, but because we get to it via
  // a using expression, it generates a UsingShadowExpr.  Make sure we
  // "see through" that properly.
  // IWYU: i1_ns::I1_NamespaceTemplateFn is...*badinc-i1.h
  I1_NamespaceTemplateFn(t);
}

template<typename T> void CallPlacementNew(T t) {
  static char buffer[sizeof(t)];
  // These should all be iwyu violations here, even though we can't be
  // *sure* some of these are actually placment-new until we get a
  // specific type for T (at template-instantiation time).
  // IWYU: operator new is...*<new>
  new (&t) int;
  // IWYU: operator new is...*<new>
  new (buffer) T();
  // IWYU: operator new is...*<new>
  new (&t) T();
}

// This is defining a class declared in badinc-i1.h, but I think it's
// correct that it's not an IWYU violation to leave out badinc-i1.h.
class I1_DefinedInCc_Class {
 public:
  // IWYU: I1_Enum is...*badinc-i1.h
  I1_Enum i1_enum;
};

// For function declarations, we don't need complete type info for
// arguments.  But we do for the return type unless we explicitly
// say we don't want to (by providing a forward-declare).
// IWYU: I1_Class is...*badinc-i1.h.*for fn return type
// IWYU: I1_Class needs a declaration
// IWYU: I2_Struct needs a declaration
I1_Class Cc_DeclareOnlyFn(I2_Struct i2_class);

// IWYU: I2_Struct needs a declaration
I3_ForwardDeclareClass Cc_DeclareOnlyFnWithFwdDecl(I2_Struct i2_class);


class Cc_DeclareOnlyClass {
  // IWYU: I1_Class needs a declaration
  Cc_DeclareOnlyClass(I1_Class);
  // IWYU: I2_Struct needs a declaration
  I3_ForwardDeclareClass DeclareOnlyFn(I2_Struct);
};

// The implicit constructor for i1tc uses I2_Class.
// IWYU: I1_Class is...*badinc-i1.h
// IWYU: I2_Class is...*badinc-i2.h
// IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
struct Cc_ImplicitConstructorStruct {
  // IWYU: I1_TemplateClass is...*badinc-i1.h
  // IWYU: I1_Class needs a declaration
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: I2_Class needs a declaration
  I1_TemplateClass<I1_Class, I2_Class> i1tc;
};

// The implicit destructor for i1tc uses I2_Class.
// IWYU: I2_Class is...*badinc-i2.h
// IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
struct Cc_ImplicitInitializerStruct {
  // The implicit initializer of i1tc creates an I2_Class in the I1_TC ctor
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: I2_Class is...*badinc-i2.h
  // IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
  Cc_ImplicitInitializerStruct() {}
  // IWYU: I1_TemplateClass is...*badinc-i1.h
  // IWYU: I1_Class needs a declaration
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: I2_Class needs a declaration
  I1_TemplateClass<I1_Class, I2_Class> i1tc;
};

// The implicit destructor for sp uses I2_Class.
// IWYU: I2_Class is...*badinc-i2.h
// IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
struct Cc_ImplicitDestructorStruct {
  Cc_ImplicitDestructorStruct() : sp() { }
  // IWYU: I2_Class needs a declaration
  H_ScopedPtr<I2_Class> sp;
};

// IWYU: EmptyDestructorClass is...*badinc-i1.h
EmptyDestructorClass::~EmptyDestructorClass() { }
// IWYU: I2_Enum is...*badinc-i2.h
// IWYU: I22 is...*badinc-i2.h
I2_Enum H_Class::ff_ = I22;
// IWYU: I2_Enum is...*badinc-i2.h
int H_Class::f(I2_Enum i2_enum) { return 1; }
// IWYU: I2_Enum is...*badinc-i2.h
/*static*/ int H_Class::static_out_of_line(I2_Enum i2_enum) {
  // IWYU: I1_Class needs a declaration
  I1_Class* i1_class;
  i1_class = NULL;
  return 1;
}
// IWYU: I2_Enum is...*badinc-i2.h
int H_Class::H_NestedStruct::nested(I2_Enum i2_enum) {
  // IWYU: I1_Class needs a declaration
  I1_Class* i1_class;
  i1_class = 0;
  return 1;
}
// IWYU: I2_Enum is...*badinc-i2.h
int H_Class::H_NestedStruct::static_nested(I2_Enum i2_enum) {
  // IWYU: I1_Class needs a declaration
  I1_Class* i1_class;
  i1_class = 0;
  return 1;
}
class H_Class::H_Class_UnusedSubdecl {};
void H_Class::DefinedInBadincCc() {}
// IWYU: I2_Class is...*badinc-i2.h
// IWYU: I2_Struct is...*badinc-i2.h
// IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
H_Pimpl_ExplicitCtorDtor::~H_Pimpl_ExplicitCtorDtor() {}

template<typename FOO> FOO H_TemplateClass<FOO>::h_template_foo_static_;
template<typename FOO>
// IWYU: I2_Enum is...*badinc-i2.h
FOO H_TemplateClass<FOO>::static_out_of_line(I2_Enum i2_enum) {
  // IWYU: I1_Class needs a declaration
  I1_Class* i1_class;
  i1_class = NULL;
  return FOO();
}
template<typename FOO>
// IWYU: I2_Enum is...*badinc-i2.h
int H_TemplateClass<FOO>::H_TplNestedStruct::tplnested(I2_Enum i2_enum) {
  // IWYU: I1_Class needs a declaration
  I1_Class* i1_class;
  i1_class = NULL;
  return 1;
}
template<typename FOO>
// IWYU: I2_Enum is...*badinc-i2.h
FOO H_TemplateClass<FOO>::H_TplNestedStruct::static_tplnested(I2_Enum i2_enum) {
  // IWYU: I1_Class needs a declaration
  I1_Class* i1_class;
  i1_class = NULL;
  return FOO();
}

// IWYU: I2_Class is...*badinc-i2.h
int I2_Class::CcFileFn() { return 1; }

// IWYU: I2_TemplateClass is...*badinc-i2.h
template<typename FOO> int I2_TemplateClass<FOO>::CcFileFn() {
  // IWYU: I2_TemplateClass is...*badinc-i2.h
  // IWYU: I2_TemplateClass needs a declaration
  // IWYU: I2_TemplateClass::I2_TemplateClass<.*> is...*badinc-i2-inl.h
  I2_TemplateClass<int>* ptr = new I2_TemplateClass<int>(42, "hi");  // NewExpr

  // IWYU: I2_TemplateClass is...*badinc-i2.h
  // IWYU: I2_TemplateClass::I2_TemplateClass<.*> is...*badinc-i2-inl.h
  // IWYU: I2_TemplateClass::~I2_TemplateClass<.*> is...*badinc-i2-inl.h
  I2_TemplateClass<int> x(42, "hi");  // CXXConstructExpr
  (void)x;

  // IWYU: I2_TemplateClass::~I2_TemplateClass<.*> is...*badinc-i2-inl.h
  // IWYU: I2_TemplateClass is...*badinc-i2.h
  delete ptr;  // CXXDeleteExpr

  // IWYU: I2_TemplateClass::~I2_TemplateClass<.*> is...*badinc-i2-inl.h
  // IWYU: I2_TemplateClass is...*badinc-i2.h
  delete this;  // CXXDeleteExpr

  // IWYU: I2_TemplateClass::InlFileTemplateClassFn is...*badinc-i2-inl.h
  // IWYU: I2_TemplateClass is...*badinc-i2.h
  return InlFileTemplateClassFn();  // CXXMemberCallExpr
}


// Let's make some global variables of various types.

// badinc.cc
Cc_Struct cc_struct;
// IWYU: I1_Class needs a declaration
// IWYU: I1_Class is...*badinc-i1.h
Cc_TypenameTemplateStruct<I1_Class> cc_typename_template_struct;
Cc_Enum cc_enum = CC1;
Cc_Subclass cc_subclass;
MultipleInheritanceSubclass cc_multipleinheritancesubclass;
// IWYU: I1_Class needs a declaration
// IWYU: I1_Class is...*badinc-i1.h
// IWYU: I11 is...*badinc-i1.h
int cc_template_function_val = Cc_TemplateFunction<I1_Class, I11>();
int cc_tricky_template_function_val =
    // IWYU: I1_Class needs a declaration
    // IWYU: kI1ConstInt is...*badinc-i1.h
    // IWYU: I12 is...*badinc-i1.h
    // IWYU: I13 is...*badinc-i1.h
    Cc_TemplateFunction<I1_Class, kI1ConstInt == 4 ? I12 : I13>();  // is I13
// IWYU: I1_Class needs a declaration
// IWYU: I1_Class is...*badinc-i1.h
int cc_tpl_newer = Cc_TemplateNewer<I1_Class>();
// IWYU: I1_Class needs a declaration
int cc_ptr_check1 = Cc_PTS<I1_Class>::IsPointer;
// IWYU: I1_Class needs a declaration
int cc_ptr_check2 = Cc_PTS<I1_Class*>::IsPointer;
// IWYU: I1_Class needs a declaration
int cc_ptr_check3 = Cc_PTS<int I1_Class::*>::IsPointer;

// badinc.h
H_Typedef h_typedef;
// TODO(csilvers): it would be nice to be able to remove this and
//   still have all tests pass.  As it is, we need this to instantiate
//   the template's underlying type, which we need to find its violations.
// We are responsible for the underlying type of this typedef because
// the .h file has signalled (by forward-declaring the underlying type,
// and not directly including it) that it doesn't want responsibility.
// IWYU: I2_TypedefOnly_Class is...*badinc-i2.h
H_I1_Class_Typedef h_i1_class_typedef;
H_I2Enum_Set h_i2enum_set;
H_I2Enum_Set* h_i2enum_set_ptr;
H_Class h_class;
H_Class h_class2(2);
H_StructPtr h_structptr;
H_TemplateClass<H_Enum> h_templateclass1(H1);
// IWYU: I1_Enum is...*badinc-i1.h
// IWYU: I11 is...*badinc-i1.h
H_TemplateClass<I1_Enum> h_templateclass2(I11);
// IWYU: I1_Struct needs a declaration
H_TemplateStruct<I1_Struct> h_template_struct;
// IWYU: I1_Struct needs a declaration
H_TemplateStructHelper<I1_Struct> h_template_struct_helper;
// IWYU: I1_Struct needs a declaration
H_TemplateStruct<I1_Struct>* h_template_struct_ptr;
// IWYU: I1_Struct needs a declaration
H_TemplateStruct<I1_Struct, Cc_Struct> h_template_struct2;
// We need full type info for i1_templateclass because we never
// fwd-declare a class with default template parameters.
// IWYU: I1_TemplateClass is...*badinc-i1.h
H_TemplateStruct<I1_TemplateClass<int> > h_template_struct_tplclass_arg;
// TODO(csilvers): this should be attributed to the .h, since it comes
// via a default template argument.
// IWYU: I2_TemplateClass::~I2_TemplateClass<.*> is...*badinc-i2-inl.h
H_TemplateTemplateClass<> h_templatetemplateclass;
// These do not need the full type for I1_Class because they're tpl params.
// IWYU: I1_Class needs a declaration
H_TypedefStruct<I1_Class>::t_type h_typdef_struct_t;
// IWYU: I1_Class needs a declaration
H_TypedefStruct<I1_Class>::pair_type h_typdef_struct_pair;
// IWYU: I2_Class is...*badinc-i2.h
// IWYU: I2_Class needs a declaration
// IWYU: I2_TemplateFn is...*badinc-i2.h
H_FunctionPtr h_functionpointer = I2_TemplateFn<I2_Class*>;
// IWYU: I2_Class is...*badinc-i2.h
// IWYU: I2_Class needs a declaration
// IWYU: I2_TemplateFn is...*badinc-i2.h
H_FunctionPtr h_functionpointer_amp = &((I2_TemplateFn<I2_Class*>));
H_FunctionPtr &h_functionpointer_ref = h_functionpointer;
// IWYU: H_Class::H_Class_DefinedInI1 is...*badinc-i1.h
H_Class::H_Class_DefinedInI1 h_class_subclass_in_i1;
// Note: H_Class::H_Class_DefinedInI1 is forward-declared in badinc.h,
// hence we get it for free.
H_Class::H_Class_DefinedInI1* h_class_subclass_in_i1_ptr;
// This requires the full type for two reasons: because of the
// H_ScopedPtr typedef and because we "own" the destructor call.
// IWYU: I1_Class needs a declaration
// IWYU: I1_Class is...*badinc-i1.h
H_ScopedPtr<I1_Class> h_scoped_ptr;
// This should also require I1_Class, because of the scoped_ptr it holds.
// IWYU: I1_Class needs a declaration
// IWYU: I1_Class is...*badinc-i1.h
H_ScopedPtrHolder<I1_Class> h_scoped_ptr_holder;

InlH_FunctionPtr inlh_functionptr = InlH_Function();  // badinc-inl.h

D1_Class d1_class;       // badinc-d1.h
D1_TemplateClass<D1_Enum> d1_templateclass1(D11);
// IWYU: I2_Enum is...*badinc-i2.h
// IWYU: I2_LAST is...*badinc-i2.h
D1_TemplateClass<I2_Enum> d1_templateclass2(I2_LAST);
// IWYU: I2_Enum is...*badinc-i2.h
D1_TemplateClass<I2_Enum> *d1_templateclass2_ptr;
// IWYU: I2_Enum is...*badinc-i2.h
D1_TemplateClass<I2_Enum> &d1_templateclass_ref = d1_templateclass2;
D1_TemplateStructWithDefaultParam<D1_Enum> d1_templatestructwithdefaultparam;
int d1_fn = D1Function();

// IWYU: D2_ForwardDeclareClass needs a declaration
D2_ForwardDeclareClass* d2_forward_declare_class;   // badinc-d2.h
D2_Subclass* d2_subclass_ptr_array[10];
// The reinterpret-cast here should mean we don't need the full type.
D2_Class& d2_classref = reinterpret_cast<D2_Class&>(*d2_subclass_ptr_array[0]);

// IWYU: d3_namespace::D3_Struct needs a declaration
d3_namespace::D3_Struct* d3_struct_ptr;   // badinc-d3.h

// Now the expected errors: badinc-i1.h and badinc_i2.h
// IWYU: I1_Enum is...*badinc-i1.h
I1_Enum i1_enum;
// IWYU: I1_Typedef is...*badinc-i1.h
I1_Typedef i1_typedef;
// IWYU: I1_Struct is...*badinc-i1.h
I1_Struct i1_struct;
// IWYU: i1_ns::I1_NamespaceStruct is...*badinc-i1.h
I1_NamespaceStruct i1_namespace_struct;
// IWYU: I1_Union is...*badinc-i1.h
I1_Union i1_union;
// IWYU: I1_UnnamedStruct is...*badinc-i1.h
I1_UnnamedStruct i1_unnamedstruct;
// Test having this ns class forward-declared first, then used fully.
// IWYU: I1_NamespaceClass needs a declaration
i1_ns::I1_NamespaceClass* i1_namespace_class_ptr;
// A declaration which includes namespace-qualification cannot serve
// as a forward-declaration.
// IWYU: i1_ns::I1_NamespaceClass needs a declaration
class i1_ns::I1_NamespaceClass* i1_namespace_class_ptr2;
// IWYU: i1_ns::I1_NamespaceClass is...*badinc-i1.h
i1_ns::I1_NamespaceClass i1_namespace_class;
// As above, a namespace-qualified friend declaration requires a
// previous full declaration.
// IWYU: I1_NamespaceClass needs a declaration
class FriendsNamespaceClass { friend class i1_ns::I1_NamespaceClass; };
namespace i1_ns {
// No forward declaration needed if no namespace-qualification.
class FriendsNamespaceClassInNS { friend class I1_NamespaceClass; };
}  // namespace i1_ns

// IWYU: I1_Class is...*badinc-i1.h
I1_Class i1_class;
// IWYU: I1_Class is...*badinc-i1.h
I1_Class i1_class_init(5);
// IWYU: I1_Class is...*badinc-i1.h
I1_Class i1_class_array[10];
// IWYU: I1_Class needs a declaration
// IWYU: kI1ConstInt is...*badinc-i1.h
I1_Class *i1_class_array_ptr[kI1ConstInt];
// IWYU: I1_Class needs a declaration
I1_Class *i1_class_ptr;
// IWYU: I1_Class needs a declaration
I1_Class &i1_class_ref = i1_class;
// IWYU: I1_Class is...*badinc-i1.h
I1_Class::NestedStruct *i1_nestedstruct_ptr;
// Even adding an explicit 'struct' doesn't make it fwd-declarable.
// IWYU: I1_Class is...*badinc-i1.h
struct I1_Class::NestedStruct *i1_nestedstruct_ptr2;
// IWYU: I1_Class::NestedStruct is...*badinc-i1.h
// IWYU: I1_Class is...*badinc-i1.h
I1_Class::NestedStruct i1_nestedstruct;
// IWYU: I1_Class::NestedStruct is...*badinc-i1.h
// IWYU: I1_Class is...*badinc-i1.h
struct I1_Class::NestedStruct i1_nestedstruct2;

// IWYU: I1_Base needs a declaration
I1_Base *i1_base_ptr;

// I1_Enum is here twice because we need it in two places: once for
// the I1_TemplateClass<I1_Enum> object, and once in the constructor.
// IWYU: I1_TemplateClass is...*badinc-i1.h
// IWYU: I1_Enum is...*badinc-i1.h
// IWYU: I11 is...*badinc-i1.h
I1_TemplateClass<I1_Enum> i1_templateclass(I11);
// IWYU: I1_TemplateClass is...*badinc-i1.h
I1_TemplateClass<int> i1_templateclass2(10);
// IWYU: I1_Class needs a declaration
// IWYU: I1_TemplateClass is...*badinc-i1.h
I1_TemplateClass<I1_Class*> i1_templateclass3(NULL);
// We need full type info for i1_templateclass because we never
// fwd-declare a class with default template parameters.
// IWYU: I1_TemplateClass is...*badinc-i1.h
// IWYU: I1_Class needs a declaration
I1_TemplateClass<I1_Class>* i1_templateclass_ptr;
// IWYU: I1_Class needs a declaration
// IWYU: I1_Class is...*badinc-i1.h
// IWYU: I1_TemplateClass is...*badinc-i1.h
I1_TemplateClass<I1_Class> i1_templateclass_object;
// IWYU: std::vector is...*<vector>
// IWYU: I1_TemplateClass is...*badinc-i1.h
// IWYU: I1_Class needs a declaration
// IWYU: I1_Class is...*badinc-i1.h
I1_TemplateClass<std::vector<I1_Class> > i1_nested_templateclass(i1_union);
// We need full type info for i1_templateclass because we never
// fwd-declare a class with default template parameters.
// IWYU: I1_TemplateClass is...*badinc-i1.h
// IWYU: I1_Class needs a declaration
// IWYU: std::vector is...*<vector>
I1_TemplateClass<std::vector<I1_Class> >* i1_nested_templateclass_ptr;
// IWYU: I1_TemplateClass is...*badinc-i1.h
// IWYU: I1_Class needs a declaration
// IWYU: std::vector is...*<vector>
I1_TemplateClass<std::vector<I1_Class>*> i1_nested_templateclass_ptr2(NULL);
// IWYU: I1_TemplateSubclass is...*badinc-i1.h
// IWYU: I1_Class needs a declaration
// IWYU: I1_Class is...*badinc-i1.h
// IWYU: I2_Class needs a declaration
// IWYU: I2_Class is...*badinc-i2.h
// IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
I1_TemplateSubclass<I1_Class, I2_Class> i1_template_subclass;
// IWYU: I1_Enum is...*badinc-i1.h
// IWYU: std::vector is...*<vector>
std::vector<I1_Enum> enum_vec;
// IWYU: std::vector is...*<vector>
// IWYU: I1_TemplateClass is...*badinc-i1.h
// IWYU: I1_Enum is...*badinc-i1.h
I1_TemplateClass<std::vector<I1_Enum> > i1_nested_templateclass_enum(enum_vec);
// We need full type info for i1_templateclass because we never
// fwd-declare a class with default template parameters.
// IWYU: I1_TemplateClass is...*badinc-i1.h
// IWYU: I1_Enum is...*badinc-i1.h
// IWYU: std::vector is...*<vector>
I1_TemplateClass<std::vector<I1_Enum> >* i1_nested_templateclass_enum_ptr;
// IWYU: I1_TemplateClass is...*badinc-i1.h
// IWYU: I1_Enum is...*badinc-i1.h
// IWYU: std::vector is...*<vector>
I1_TemplateClass<std::vector<I1_Enum>*> i1_nested_templateclass_enum_ptr2(NULL);
// Note: we have to avoid the most vexing parse, here!
// IWYU: I1_TemplateClass is...*badinc-i1.h
I1_TemplateClass<D1_I1_Typedef> i1_tplclass_with_typedef((D1_I1_Typedef()));
// IWYU: I1_TemplateClass is...*badinc-i1.h
I1_TemplateClass<D1_I1_Typedef*> i1_templateclass_with_typedef_ptr(NULL);
// IWYU: I1_TemplateClass is...*badinc-i1.h
I1_TemplateClass<D1_Enum> i1_templateclass_d(D11);
// I1_Class is needed as a used template arg to this template-constructor.
// IWYU: I1_Class is...*badinc-i1.h
// IWYU: I2_Struct needs a declaration
// IWYU: I2_Struct is...*badinc-i2.h
// IWYU: I1_TemplateClass is...*badinc-i1.h
I1_TemplateClass<I2_Struct> i1_templateclass_tpl_ctor(i1_class, true);
// Accessing a macro in the same file should still cause an iwyu warning.
// The trick here is that the NNS doesn't have its own location info.
#define CC_DEFINE_VAR(typ)  typ cc_define_var ## __LINE__
// IWYU: I1_TemplateClass is...*badinc-i1.h
// IWYU: I1_TemplateClass<.*>::I1_TemplateClass_int is...*badinc-i1.h
// IWYU: I2_Class is...*badinc-i2.h
// IWYU: I2_Class needs a declaration
CC_DEFINE_VAR(I1_TemplateClass<I2_Class>::I1_TemplateClass_int);

// IWYU: I1_TemplateMethodOnlyClass is...*badinc-i1.h
// IWYU: I1_Class needs a declaration
I1_TemplateMethodOnlyClass<I1_Class> i1_template_method_only_class;
// IWYU: I2_TemplateClass needs a declaration
// IWYU: I1_TemplateMethodOnlyClass is...*badinc-i1.h
// IWYU: I1_Class needs a declaration
I1_TemplateMethodOnlyClass<I2_TemplateClass<const I1_Class> > i1_tpl_tpl_class;
// IWYU: I1_TemplateClassFwdDeclaredInD2 needs a declaration
I1_TemplateClassFwdDeclaredInD2<int, float>* i1_tpl_class_fwd_declared_in_d2;
// IWYU: I1_Class needs a declaration
// IWYU: I1_Class is...*badinc-i1.h
// IWYU: I1_TypedefOnly_Class is...*badinc-i1.h
I1_TypedefOnly_Class<I1_Class> i1_typedefonly_class;
// IWYU: I1_Class needs a declaration
// IWYU: I1_Class is...*badinc-i1.h
// IWYU: I1_TypedefOnly_Class is...*badinc-i1.h
// IWYU: I1_TypedefOnly_Class<.*>::i is...*badinc-i1.h
I1_TypedefOnly_Class<I1_Class>::i i1_typedefonly_class_int;
// IWYU: I1_I2_Class_Typedef is...*badinc-i1.h
int i1_i2_class_typedef_int = I1_I2_Class_Typedef::s;

// IWYU: I1_Subclass is...*badinc-i1.h
I1_Subclass i1_subclass;

// IWYU: I1_ManyPtrStruct needs a declaration
I1_ManyPtrStruct ***i1_manyptrstruct_ptr_ptr_ptr;
// IWYU: I1_ClassPtr is...*badinc-i1.h
I1_ClassPtr i1_classptr;
// IWYU: I1_FunctionPtr is...*badinc-i1.h
// IWYU: I1_Function is...*badinc-i1.h
I1_FunctionPtr i1_functionptr = I1_Function;
// IWYU: I1_FunctionPtr is...*badinc-i1.h
I1_FunctionPtr *i1_functionptr_ptr = &i1_functionptr;
// IWYU: I1_ForwardDeclareClass needs a declaration
I1_ForwardDeclareClass* i1_forwarddeclareptr;
// IWYU: I1_Function is...*badinc-i1.h
int i1_function_value = I1_Function(0);

// Two different uses of an indirect global.  The first is a function call
// which is exposed in the compiler-generated function for global
// constructors/destructors.  The second is similar, but it requires no
// call since its a simple initialization.
// IWYU: i1_GlobalFunction is...*badinc-i1.h
int i1_globalfunctionval = i1_GlobalFunction();
// IWYU: i1_GlobalFunction is...*badinc-i1.h
int (*i1_globalfunctionptr)(void) = i1_GlobalFunction;
// IWYU: i1_GlobalFunction is...*badinc-i1.h
int (*i1_globalfunctionptr2)(void) = &i1_GlobalFunction;

// IWYU: I2_InlFileClass is...*badinc-i2-inl.h
I2_InlFileClass i2_inlfileclass;
// IWYU: I2_InlFileTemplateClass is...*badinc-i2-inl.h
I2_InlFileTemplateClass<int> i2_inlfiletemplateclass;
// IWYU: I2_Class...*badinc-i2.h
// IWYU: I2_Class::I2_Class...*badinc-i2-inl.h
// IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
I2_Class i2_class_with_inl_constructor("calling ctor in badinc-i2-inl.h");
// IWYU: I2_TemplateClass...*badinc-i2.h
// IWYU: I2_TemplateClass::I2_TemplateClass<.*> is...*badinc-i2-inl.h
// IWYU: I2_TemplateClass::~I2_TemplateClass<.*> is...*badinc-i2-inl.h
I2_TemplateClass<int> i2_template_class_with_inl_constructor(4, "inl ctor");
// We don't need a fwd-decl of I2_TypedefOnly_Class; we get that via badinc.h
// IWYU: I1_Class needs a declaration
I2_TypedefOnly_Class<I1_Class>* i1_typedefonly_class_ptr;

// Not an iwyu violation because I explicitly forward-declared this class.
I3_ForwardDeclareClass* i3_forwarddeclareclass;
// IWYU: I3_ForwardDeclareStruct needs a declaration
I3_ForwardDeclareStruct* i3_forwarddeclarestruct;
// Not an iwyu violation because I explicitly forward-declared this class.
I3_SimpleForwardDeclareTemplateStruct<int>* i3_simpleforwarddeclaretplstruct;
// IWYU: I3_ForwardDeclareTemplateStruct needs a declaration
I3_ForwardDeclareTemplateStruct<int, 4, 'b'>* i3_forwarddeclaretemplatestruct;
// Not an iwyu violation because I explicitly forward-declared this class.
i3_ns1::i3_ns2::i3_ns3::I3_ForwardDeclareNamespaceStruct* i3_fdns_struct;
// IWYU: I3_ForwardDeclareNamespaceTemplateStruct needs a declaration
i3_ns1::i3_ns2::i3_ns3::I3_ForwardDeclareNamespaceTemplateStruct<H_Enum, 2>*
  i3_fdtns_struct;

// IWYU: I3_UnnamedNamespaceStruct needs a declaration
i3_ns1::I3_UnnamedNamespaceStruct* i3_unnamed_namespace_struct;

// This shouldn't cause weird iwyu issues between us and badinc.h.
H_USE_CLASS(MacroClass);
// This shouldn't cause badinc.h to need to include us for Cc_Subclass.
H_CREATE_VAR(Cc_Subclass);
// We should be credited for use of I1_Class even though it's in the macro.
// IWYU: I1_Class is...*badinc-i1.h
int identity_i1_class_a = H_IDENTITY(i1_class).a();

// Violations within a namespace (both named an unnamed)
namespace cc_ns {
// IWYU: I1_Typedef is...*badinc-i1.h
I1_Typedef i1_typedef;
}
namespace {
// IWYU: I1_Union is...*badinc-i1.h
I1_Union i1_unnamed_union;
}

// I3_ForwardDeclareClass is not an iwyu violation, because I
// explicitly forward-declare it above.
// IWYU: I3_ForwardDeclareStruct needs a declaration
// IWYU: I1_Class needs a declaration
float SimpleFunction(I3_ForwardDeclareClass* a, I3_ForwardDeclareStruct* b)
    throw(I3_ForwardDeclareClass*, I1_Class*);

// IWYU: I3_ForwardDeclareStruct needs a declaration
// IWYU: I1_Class needs a declaration
float SimpleFunction(I3_ForwardDeclareClass* a, I3_ForwardDeclareStruct* b)
    throw(I3_ForwardDeclareClass*, I1_Class*) {
  return 1.0;
}
float simple_function = SimpleFunction(NULL, NULL);

// IWYU: I3_ForwardDeclareStruct needs a declaration
template<bool Foo> int SimpleConstantTemplateFunction()
    throw(I3_ForwardDeclareStruct&) {
  return Foo;
}
bool simple_constant_template_function = SimpleConstantTemplateFunction<5<3>();

// Let's test the code that distinguishes pointers (and references), a bit.
// IWYU: I3_ForwardDeclareStruct needs a declaration
int SimpleUnusedFunction(const I3_ForwardDeclareStruct& a) {
  return 1;
}
// IWYU: I3_ForwardDeclareStruct needs a declaration
int SimpleUnusedFunction2(I3_ForwardDeclareStruct* a) {
  return 2;
}
// IWYU: I3_ForwardDeclareStruct needs a declaration
int SimpleUnusedFunction3(I3_ForwardDeclareStruct& a,
                          // IWYU: I3_ForwardDeclareStruct needs a declaration
                          I3_ForwardDeclareStruct* b,
                          // IWYU: I3_ForwardDeclareStruct needs a declaration
                          I3_ForwardDeclareStruct** c) {
  return (SimpleUnusedFunction(a) +
          SimpleUnusedFunction(*b) +
          SimpleUnusedFunction(**c) +
          SimpleUnusedFunction2(*c));
}

// Test that we properly detect we need badinc-d4 for operator<<
// (even though we only need a forward-declare for D4_ClassForOperator).
int TestOperator(const D4_ClassForOperator& d4_classop) {
  return 1 << d4_classop;
}

// Test we properly find both uses (one ptr-only and one not) when on same line.
// IWYU: I1_PtrAndUseOnSameLine is...*badinc-i1.h
// IWYU: I1_PtrAndUseOnSameLine needs a declaration
int TestSameLine(I1_PtrAndUseOnSameLine* i1_ptruse) { return i1_ptruse->a(); }

// Test casting: try passing in an I1_SubclassesI2Class* here.
// IWYU: I2_Class needs a declaration
void FunctionTakesI2_Class(I2_Class* d2c) { }

template class D1_TemplateClass<D1_StructPtr>;  // instantiate a few templates
// IWYU: I1_ClassPtr is...*badinc-i1.h
template class D1_TemplateClass<I1_ClassPtr>;
// IWYU: I1_TemplateClass is...*badinc-i1.h
template class I1_TemplateClass<int>;
// IWYU: I1_TemplateClass is...*badinc-i1.h
// IWYU: I1_ClassPtr is...*badinc-i1.h
template class I1_TemplateClass<I1_ClassPtr>;

// Test typedefs on various types of instantiations.
// First, make sure the implicit instantiation instantiates some methods too.
// IWYU: I2_TemplateClass is...*badinc-i2.h
int i2_tpl_class_a = i2_template_class_with_inl_constructor.a();
// IWYU: I2_TemplateClass is...*badinc-i2.h
// IWYU: I2_TemplateClass::I2_TemplateClass<.*> is...*badinc-i2-inl.h
// IWYU: I2_TemplateClass::~I2_TemplateClass<.*> is...*badinc-i2-inl.h
// IWYU: I2_TemplateClass::InlFileTemplateClassFn is...*badinc-i2-inl.h
typedef I2_TemplateClass<int> Cc_typedef_implicit_instantiation;
// Make sure we can do the same typedef multiple times.
// IWYU: I2_TemplateClass is...*badinc-i2.h
// IWYU: I2_TemplateClass::I2_TemplateClass<.*> is...*badinc-i2-inl.h
// IWYU: I2_TemplateClass::~I2_TemplateClass<.*> is...*badinc-i2-inl.h
// IWYU: I2_TemplateClass::InlFileTemplateClassFn is...*badinc-i2-inl.h
typedef I2_TemplateClass<int> Cc_typedef_implicit_instantiation2;
// IWYU: I1_ClassPtr is...*badinc-i1.h
// IWYU: I1_TemplateClass is...*badinc-i1.h
typedef I1_TemplateClass<I1_ClassPtr> Cc_typedef_explicit_instantiation;
// IWYU: I1_ClassPtr is...*badinc-i1.h
// IWYU: I1_TemplateClass is...*badinc-i1.h
typedef I1_TemplateClass<I1_ClassPtr> Cc_typedef_explicit_instantiation2;
// IWYU: I1_TemplateClass is...*badinc-i1.h
typedef I1_TemplateClass<char> Cc_typedef_no_previous_instantiation;
// IWYU: I1_TemplateClass is...*badinc-i1.h
typedef I1_TemplateClass<char> Cc_typedef_no_previous_instantiation2;

class Cc_Class {
  friend class I2_Class;
  // IWYU: I1_Class is...*badinc-i1.h
  friend struct I1_Class::NestedStruct;
  template<class T> friend class I2_TemplateClass;
  // IWYU: I2_TemplateClass needs a declaration
  friend class I2_TemplateClass<int>;
  // IWYU: I1_Class is...*badinc-i1.h
  template<class T> friend struct I1_Class::NestedTemplateStruct;
};

void Cc_ForwardDeclare_Function() {
  class I2_Class;
}

// This class template fully uses its type parameter.
template <typename T> class Cc_TemplateFullyUse {
 private:
  T value_;
};

// This class template doesn't fully use its type parameter.
template <typename T> class Cc_TemplatePartiallyUse {
 private:
  Cc_TemplateFullyUse<T>* ptr_;  // shouldn't require T's full type
};

// IWYU: I1_Class needs a declaration
Cc_TemplatePartiallyUse<I1_Class> cc_template_class1;

template <typename T>
class Cc_TemplateDerived
    : public Cc_TemplateFullyUse<T> {  // test traversing the base class
};

// IWYU: I1_Class needs a declaration
// IWYU: I1_Class is...*badinc-i1.h
Cc_TemplateDerived<I1_Class> cc_template_derived1;

// The following three classes test the fix for http://llvm.org/PR8403.

template <class A>
struct CC_TemplateStruct2 { };

class CC_Class2 {
  struct CC_TemplateStruct2<int> x;
};

// We require full type info for default template parameters, even
// when it's not strictly necessary, just to avoid potential trouble.
// (Users shouldn't have to worry about instantiating default params.)
// IWYU: I1_Class is...*badinc-i1.h
template <class A, class B = I1_Class>
class CC_TemplateClass {
 public:
  struct CC_TemplateStruct2<A> x;
  // IWYU: I1_TemplateClass is...*badinc-i1.h
  typedef I1_TemplateClass<A> i1_typedef;

  // Let's throw in per-class operator new/delete.
  // IWYU: size_t is...*<stddef.h>
  void* operator new(size_t size) {
    B b;
    (void)b;
    return ::operator new(4);
  }
  void operator delete(void* p) {
    A a;
    (void)a;
    ::operator delete(p);
  }
  // Test deleting a dependent type
  void DeleteAnA() {
    A a; delete a;   // A had better be a pointer type!
  }
};

// This should stay untouched by iwyu.
#include "tests/cxx/badinc2.c"

int main() {
  // Some of the above as local variables as well
  Cc_Struct local_cc_struct = {};
  Cc_string local_cc_string;
  H_Class local_h_class;
  // IWYU: I2_Enum is...*badinc-i2.h
  // IWYU: I21 is...*badinc-i2.h
  D1_TemplateClass<I2_Enum> local_d1_template_class(I21);
  // IWYU: I1_Enum is...*badinc-i1.h
  // IWYU: I11 is...*badinc-i1.h
  I1_Enum local_i1_enum = I11;
  // IWYU: I1_UnnamedStruct is...*badinc-i1.h
  I1_UnnamedStruct local_i1_unnamed_struct = {};
  D1_Subclass local_d1_subclass;
  // IWYU: I2_Class needs a declaration
  I2_Class* local_i2_class_ptr = 0;  // ptr-only in this .cc, non-ptr in .h
  // IWYU: I2_TemplateClass...*badinc-i2.h
  // IWYU: I2_TemplateClass::~I2_TemplateClass<.*> is...*badinc-i2-inl.h
  I2_TemplateClass<int> local_i2_template_class(1);
  // IWYU: I1_PtrDereferenceStruct needs a declaration
  const I1_PtrDereferenceStruct* const local_i1_ptrdereference_struct = 0;
  // IWYU: I1_PtrDereferenceClass needs a declaration
  I1_PtrDereferenceClass* local_i1_ptrdereference_class = 0;
  int x;
  // IWYU: va_list is...*<stdarg.h>
  va_list vl;  // in gcc, va_list is an internal type, so this tests <built-in>
  D1_I1_Typedef d1_i1_typedef;
  // IWYU: i1_int is...*badinc-i1.h
  int vararray[i1_int];
  std::fstream fs;

  isascii('a');   // declared in <ctype.h> which is #included by badinc.h
  (void)(errno);  // declared in <errno.h> which is #included by badinc.h
  // Test we don't give an iwyu warning for the built-in __builtin_expect
  // IWYU: I11 is...*badinc-i1.h
  if (__builtin_expect(local_i1_enum == I11, false)) (void)local_i1_enum;

  // IWYU: i1_int_global is...*badinc-i1.h
  x = i1_ns::i1_int_global;
  // IWYU: i1_ns::i1_subns::i1_int_globalsub is...*badinc-i1.h
  x = i1_ns::i1_subns::i1_int_globalsub;
  // IWYU: i1_ns2::i1_int_global2 is...*badinc-i1.h
  x = i1_int_global2;
  // IWYU: i1_ns2::i1_subns::i1_int_global2sub is...*badinc-i1.h
  x = i1_subns::i1_int_global2sub;
  // IWYU: i1_ns3::i1_int_global3 is...*badinc-i1.h
  x = i1_int_global3;
  // IWYU: i1_ns3::i1_subns::i1_int_global3sub is...*badinc-i1.h
  x = i1_ns3::i1_subns::i1_int_global3sub;
  // IWYU: i1_ns4::i1_int_global4 is...*badinc-i1.h
  x = cc_ns_alias::i1_int_global4;
  // IWYU: i1_ns4::i1_subns::i1_int_global4sub is...*badinc-i1.h
  x = cc_ns_alias::i1_subns::i1_int_global4sub;

  // Reference all the local variables, to avoid not-used errors.
  // IWYU: fprintf is...*<stdio.h>
  // IWYU: stdout is...*<stdio.h>
  fprintf(stdout, "%d", local_cc_struct.a);  // test a global in stdio.h too
  // IWYU: printf is...*<stdio.h>
  printf("%d", local_cc_struct.a);
  (void)Cc_typedef(4);
  // Not an iwyu violation, because Cc_typedef is responsible for its members.
  Cc_typedef::s();
  Cc_typedef::I1_Class_int Cc_typedef_int = 0;
  Cc_typedef::NestedStruct Cc_typedef_struct;
  Cc_typedef::NestedStruct::NestedStructTypedef Cc_typedef_nested_int = 0;
  local_h_class.a();
  // a() returns a FOO, which in this case is I2_Enum.
  // IWYU: I2_Enum is...*badinc-i2.h
  local_d1_template_class.a();
  (void)(local_i1_enum);
  // IWYU: I1_UnnamedStruct is...*badinc-i1.h
  (void)(local_i1_unnamed_struct.a);
  local_d1_subclass.a();
  (void)(local_i2_class_ptr);
  // IWYU: I1_PtrDereferenceStruct is...*badinc-i1.h
  (void)(local_i1_ptrdereference_struct->a);
  // Make the last ref to local_i1_ptrdereference_struct
  // fwd-declarable.  This tests that if we do non-fwd-decl-use
  // followed by fwd-decl-use, we correctly say it can't be fwd-declared.
  (void)(local_i1_ptrdereference_struct);
  // IWYU: I1_PtrDereferenceClass is...*badinc-i1.h
  local_i1_ptrdereference_class->a();

  // Calling an overloaded function.  In the first two cases,
  // CallOverloadedFunctionSameFile() is responsible for the call,
  // since it's just a single file.  In the second two cases, we
  // can't know the file required until now (when the templated
  // function is instantiated).
  CallOverloadedFunctionSameFile(5);
  CallOverloadedFunctionSameFile(5.0f);
  // IWYU: I1_And_I2_OverloadedFunction is...*badinc-i1.h
  CallOverloadedFunctionDifferentFiles(5);
  // IWYU: I1_And_I2_OverloadedFunction is...*badinc-i2.h
  CallOverloadedFunctionDifferentFiles(5.0f);
  // This should not be an IWYU violation either: the iwyu use is in the fn.
  CallOverloadWithUsingShadowDecl(5);
  // IWYU: I1_Class is...*badinc-i1.h
  CallPlacementNew(i1_class);

  // Calling operator<< when the first argument is a macro.  We should
  // still detect that operator<< is being used here, and not in the
  // macro-definition file.
  // IWYU: I1_MACRO_LOGGING_CLASS is...*badinc-i1.h
  // IWYU: I2_OperatorDefinedInI1Class::operator<< is...*badinc-i1.h
  I1_MACRO_LOGGING_CLASS << 1;

  // Calling a template method can cause the template arg to be non-fwd-decl.
  // IWYU: I1_TemplateMethodOnlyClass is...*badinc-i1.h
  // IWYU: I1_Class is...*badinc-i1.h
  i1_template_method_only_class.a();
  // IWYU: I1_TemplateMethodOnlyClass is...*badinc-i1.h
  i1_template_method_only_class.b();
  // IWYU: I1_TemplateMethodOnlyClass is...*badinc-i1.h
  // IWYU: I2_Class needs a declaration
  // IWYU: I2_Class is...*badinc-i2.h
  // IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
  i1_template_method_only_class.c<I2_Class>();
  // IWYU: I1_TemplateMethodOnlyClass is...*badinc-i1.h
  // IWYU: I2_Class needs a declaration
  i1_template_method_only_class.c<I2_Class*>();
  // IWYU: I1_TemplateMethodOnlyClass is...*badinc-i1.h
  // IWYU: I2_Class needs a declaration
  i1_template_method_only_class.d<I2_Class>();
  // IWYU: I1_TemplateMethodOnlyClass is...*badinc-i1.h
  // IWYU: I2_Class needs a declaration
  i1_template_method_only_class.d<I2_Class*>();
  // IWYU: I1_TemplateMethodOnlyClass is...*badinc-i1.h
  // IWYU: I2_Class is...*badinc-i2.h
  i1_template_method_only_class.e(local_i2_class_ptr);
  // IWYU: std::vector is...*<vector>
  // IWYU: I2_Class needs a declaration
  std::vector<I2_Class>* i2_class_vector = NULL;
  // IWYU: I2_Class is...*badinc-i2.h
  // IWYU: I1_TemplateMethodOnlyClass is...*badinc-i1.h
  // IWYU: std::vector is...*<vector>
  i1_template_method_only_class.e(i2_class_vector);
  // Four different ways of making the same function call.
  // IWYU: I1_TemplateMethodOnlyClass is...*badinc-i1.h
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: I2_Class is...*badinc-i2.h
  i1_template_method_only_class.s(local_i2_class_ptr);
  // IWYU: I1_TemplateMethodOnlyClass is...*badinc-i1.h
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: I2_Class needs a declaration
  // IWYU: I2_Class is...*badinc-i2.h
  i1_template_method_only_class.s<I2_Class*>(local_i2_class_ptr);
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: I1_Class needs a declaration
  // IWYU: I1_TemplateMethodOnlyClass is...*badinc-i1.h
  // IWYU: I2_Class is...*badinc-i2.h
  I1_TemplateMethodOnlyClass<I1_Class>::s(local_i2_class_ptr);
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: I1_Class needs a declaration
  // IWYU: I1_TemplateMethodOnlyClass is...*badinc-i1.h
  // IWYU: I2_Class needs a declaration
  // IWYU: I2_Class is...*badinc-i2.h
  I1_TemplateMethodOnlyClass<I1_Class>::s<I2_Class*>(local_i2_class_ptr);
  // IWYU: I1_Class needs a declaration
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: I2_Class needs a declaration
  // IWYU: I1_TemplateMethodOnlyClass is...*badinc-i1.h
  I1_TemplateMethodOnlyClass<I2_Class>::t<I1_Class>();
  // IWYU: I1_TemplateClass is...*badinc-i1.h
  // IWYU: I2_Class needs a declaration
  // IWYU: I1_TemplateMethodOnlyClass is...*badinc-i1.h
  I1_TemplateMethodOnlyClass<I2_Class>::tt<I1_TemplateClass>();
  // Try the static calls again, but this time with a typedef tpl arg.
  // IWYU: I1_TemplateMethodOnlyClass is...*badinc-i1.h
  // IWYU: I2_Class is...*badinc-i2.h
  I1_TemplateMethodOnlyClass<Cc_typedef>::s(local_i2_class_ptr);
  // IWYU: I1_TemplateMethodOnlyClass is...*badinc-i1.h
  // IWYU: I2_Class needs a declaration
  // IWYU: I2_Class is...*badinc-i2.h
  I1_TemplateMethodOnlyClass<Cc_typedef>::s<I2_Class*>(local_i2_class_ptr);

  // The result of static_cast depends on the relation between the
  // source and target type, and thus requires the full target type
  // whenever source and target types aren't the same.
  // IWYU: I1_Class needs a declaration
  (void)(static_cast<I1_Class*>(i1_class_ptr));     // same type
  // IWYU: I1_Class needs a declaration
  (void)(static_cast<I1_Class**>(&i1_class_ptr));   // same type
  // IWYU: I1_Base needs a declaration
  // IWYU: I1_Class is...*badinc-i1.h
  (void)(static_cast<I1_Base*>(i1_class_ptr));
  // IWYU: I1_Base needs a declaration
  // IWYU: I1_Class is...*badinc-i1.h
  (void)(static_cast<I1_Base&>(i1_class_ref));
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: I1_Base needs a declaration
  (void)(static_cast<I1_Base&>(i1_class));
  // IWYU: I1_Class needs a declaration
  // IWYU: I1_Class is...*badinc-i1.h
  (void)(static_cast<I1_Class*>(i1_base_ptr));
  // Full type information isn't needed since 0 isn't a pointer.
  // IWYU: I1_Class needs a declaration
  (void)(static_cast<I1_Class*>(0));
  // IWYU: I1_Class needs a declaration
  (void)(static_cast<I1_Class**>(0));

  // For implicit cast, we need to know whether the source type is
  // derived from the target type, and thus need the full type of the
  // former.
  // IWYU: I1_Class is...*badinc-i1.h
  i1_base_ptr = i1_class_ptr;
  // IWYU: I1_SubclassesI2Class needs a declaration
  I1_SubclassesI2Class* i1_subclasses_i2_ptr = 0;
  // IWYU: I1_SubclassesI2Class is...*badinc-i1.h
  FunctionTakesI2_Class(i1_subclasses_i2_ptr);
  // We don't need to know full type info if one pointer is a built-in type.
  void* void_ptr = i1_class_ptr;
  // Also tests reference-casts.
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: I1_Base needs a declaration
  const I1_Base& i1_base_implicit_cast1 = i1_class_ref;
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: I1_Base needs a declaration
  const I1_Base& i1_base_implicit_cast2 = i1_class;

  // dynamic_cast requires the full type of the source and target.
  // They may be siblings, which is why we can't say the target
  // brings in the source, like we can for a normal down-cast.
  // IWYU: I1_Base is...*badinc-i1.h
  // IWYU: I1_Class needs a declaration
  // IWYU: I1_Class is...*badinc-i1.h
  i1_class_ptr = dynamic_cast<I1_Class*>(i1_base_ptr);
  // IWYU: I1_SiblingClass needs a declaration
  I1_SiblingClass* i1_sibling_class_ptr = 0;
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: I1_SiblingClass needs a declaration
  // IWYU: I1_SiblingClass is...*badinc-i1.h
  i1_sibling_class_ptr = dynamic_cast<I1_SiblingClass*>(i1_class_ptr);

  // const_cast doesn't require the full type.
  // IWYU: I1_Class needs a declaration
  const I1_Class* const_i1_class_ptr = 0;
  // IWYU: I1_Class needs a declaration
  i1_class_ptr = const_cast<I1_Class*>(const_i1_class_ptr);

  // reinterpret_cast doesn't require the full type.
  // IWYU: I1_Class needs a declaration
  i1_class_ptr = reinterpret_cast<I1_Class*>(i1_base_ptr);

  // C-style cast doesn't require the full type either, according
  // to the language, but we ask for full type when it's an up-cast
  // or a down-cast.
  // IWYU: I1_Class needs a declaration
  // IWYU: I1_Class is...*badinc-i1.h
  i1_class_ptr = (I1_Class*)(i1_base_ptr);

  // Constructor casts require the full to-type (to call its constructor).
  // IWYU: I2_Struct is...*badinc-i2.h
  I2_Struct ctor_cast_struct_nonref;
  // IWYU: I2_Struct needs a declaration
  I2_Struct& ctor_cast_struct = ctor_cast_struct_nonref;
  // IWYU: I2_Union is...*badinc-i2.h
  I2_Union ctor_cast_union_nonref;
  // IWYU: I2_Union needs a declaration
  I2_Union& ctor_cast_union = ctor_cast_union_nonref;
  // IWYU: I2_Class is...*badinc-i2.h
  // IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
  I2_Class i2_class_from_struct = ctor_cast_struct;   // ctor takes a reference
  // IWYU: I2_Class is...*badinc-i2.h
  // IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
  I2_Class i2_class_from_union = ctor_cast_union;   // ctor takes a value

  // User-defined casts need the full from-type (to call its operator totype()).
  // IWYU: I2_Class is...*badinc-i2.h
  // IWYU: I2_UnionFunction is...*badinc-i2.h
  I2_UnionFunction(*local_i2_class_ptr);

  // Even dereferencing doesn't require the full type.
  (void)(*i1_class_ptr);

  // IWYU: I1_PtrDereferenceStatic is...*badinc-i1.h
  (void)(I1_PtrDereferenceStatic::a);
  // IWYU: I1_StaticMethod is...*badinc-i1.h
  I1_StaticMethod::a();

  // Reference some of the global variables.
  (void)(__PRETTY_FUNCTION__);
  (void)(cc_struct.b);
  d1_class.a();
  (void)(cc_struct.b);
  (void)(cc_subclass.a());
  // IWYU: OperateOn is...*badinc-i1.h
  h_template_struct.a();
  // This tests a bug in clang where an implicit template instantiation
  // of a partial specialization gave the wrong location information.
  // In this case, OperateOn<I1_TemplateClass<T> > is in badinc-i1.h,
  // which is what we should report, *not* the OperateOn<T> in badinc.h.
  // IWYU: OperateOn is...*badinc-i1.h
  h_template_struct_tplclass_arg.a();
  h_scoped_ptr.get();
  // Not an iwyu violation, since we never use the dereferenced type.
  (void)(*h_scoped_ptr);
  // IWYU: I1_Class is...*badinc-i1.h ?
  (*h_scoped_ptr).a();
  // IWYU: I1_Class is...*badinc-i1.h
  h_scoped_ptr->a();
  // IWYU: I12 is...*badinc-i1.h
  D1Function(I12);
  // TODO(csilvers): should we be warning about I2_Struct?
  // IWYU: I1_Union is...*badinc-i1.h
  // IWYU: I1_Struct is...*badinc-i1.h
  (void)(i1_union.a->a);
  // IWYU: I1_Class is...*badinc-i1.h
  I1_Class::s();
  // IWYU: I2_Struct is...*badinc-i2.h
  // IWYU: I2_Function is...*badinc-i2.h
  (void)(I2_Function(local_i2_class_ptr).b);
  // IWYU: I2_Class is...*badinc-i2.h
  // IWYU: I2_Class::InlFileFn is...*badinc-i2-inl.h
  local_i2_class_ptr->InlFileFn();
  // IWYU: I2_Class is...*badinc-i2.h
  // IWYU: I2_Class::InlFileStaticFn is...*badinc-i2-inl.h
  local_i2_class_ptr->InlFileStaticFn();
  // IWYU: I2_Class is...*badinc-i2.h
  // IWYU: I2_Class::InlFileStaticFn is...*badinc-i2-inl.h
  I2_Class::InlFileStaticFn();
  // IWYU: I2_Class is...*badinc-i2.h
  // IWYU: I2_Class::InlFileTemplateFn is...*badinc-i2-inl.h
  local_i2_class_ptr->InlFileTemplateFn<int>();
  // IWYU: I2_Class is...*badinc-i2.h
  local_i2_class_ptr->AnotherTranslationUnitFn();
  // IWYU: I2_Class is...*badinc-i2.h
  local_i2_class_ptr->CcFileFn();
  // IWYU: I2_TemplateClass is...*badinc-i2.h
  // IWYU: I2_TemplateClass::InlFileTemplateClassFn is...*badinc-i2-inl.h
  local_i2_template_class.InlFileTemplateClassFn();
  // IWYU: I2_TemplateClass is...*badinc-i2.h
  local_i2_template_class.a();
  // TODO(csilvers): these first three errors are wrong, due to a bug
  // in IntendsToProvide.  The file defining this template method
  // *should* be providing badinc-i2-inl.h, but isn't (iwyu will
  // suggest we add it).  So we don't detect that that file is
  // responsible for these symbols, and not us.
  // IWYU: I2_TemplateClass::I2_TemplateClass<.*> is...*badinc-i2-inl.h
  // IWYU: I2_TemplateClass::~I2_TemplateClass<.*> is...*badinc-i2-inl.h
  // IWYU: I2_TemplateClass::InlFileTemplateClassFn is...*badinc-i2-inl.h
  // IWYU: I2_TemplateClass is...*badinc-i2.h
  local_i2_template_class.CcFileFn();
  // IWYU: InlFileFreeFn is...*badinc-i2-inl.h
  InlFileFreeFn();
  // IWYU: InlFileFreeTemplateFn is...*badinc-i2-inl.h
  InlFileFreeTemplateFn<float>();
  // IWYU: InlFileFreeTemplateFn is...*badinc-i2-inl.h
  InlFileFreeTemplateFn<int>();     // a specialization
  // IWYU: inlfile_var is...*badinc-i2-inl.h
  (void)(inlfile_var);
  // TODO(csilvers): IWYU: I1_FunctionPtr is...*badinc-i1.h
  (**i1_functionptr_ptr)(&i1_class);
  (void)(x);
  (void)(i1_class_array);
  d1_i1_typedef.a();
  D1_I1_Typedef().a();
  MACRO_CALLING_I4_FUNCTION;
  // IWYU: MACRO_CALLING_I6_FUNCTION is...*badinc-i1.h
  MACRO_CALLING_I6_FUNCTION;
  // IWYU: kI1ConstInt is...*badinc-i1.h
  (void)(kI1ConstInt);
  // IWYU: kI1ConstInt is...*badinc-i1.h
  (void)(CONCAT(kI1C, onstInt));
  // IWYU: I1_Class needs a declaration
  CONCAT(I1_, Class) *i1_concat_class_ptr;   // also test something fwd-decl
  (void)(i1_concat_class_ptr);

  Cc_string().length();

  // IWYU: kI1ConstInt is...*badinc-i1.h
  (void)(sizeof(kI1ConstInt));
  // IWYU: I1_Class is...*badinc-i1.h
  (void)(sizeof(I1_Class));
  // IWYU: I1_Class is...*badinc-i1.h
  typeof(I1_Class) another_i1_class;
  (void)(another_i1_class);
  // IWYU: I1_Class needs a declaration
  typeof(I1_Class*) another_i1_class_ptr;
  (void)(another_i1_class_ptr);
  // IWYU: kI1ConstInt is...*badinc-i1.h
  typeof(kI1ConstInt) another_const_int = 1;
  (void)(another_const_int);
  // This is a C standard macro, but is implemented via a gcc extension too.
  // IWYU: offsetof is...*<stddef.h>
  // IWYU: I1_Struct is...*badinc-i1.h
  (void)(offsetof(I1_Struct, c));
  // IWYU: kI1ConstInt is...*badinc-i1.h
  typeid(kI1ConstInt).name();
  // IWYU: I1_Class is...*badinc-i1.h
  typeid(I1_Class).name();
  // This is an extension for gcc and msvc.
  // IWYU: I1_Class is...*badinc-i1.h
  (void)(__is_enum(I1_Class));

  // Check out template iwyu determinations.
  // IWYU: I1_Class needs a declaration
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: I1_const_ptr is...*badinc-i1.h
  I1_const_ptr<I1_Class> local_i1_const_ptr(NULL);
  // Needs I1_const_ptr because it calls its operator*().
  // IWYU: I1_const_ptr is...*badinc-i1.h
  (void)(*local_i1_const_ptr);
  // IWYU: I1_const_ptr is...*badinc-i1.h
  // IWYU: I1_Class is...*badinc-i1.h
  local_i1_const_ptr->a();
  // IWYU: I1_const_ptr is...*badinc-i1.h
  // IWYU: I1_Class is...*badinc-i1.h
  local_i1_const_ptr.del();
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: I1_const_ptr is...*badinc-i1.h
  local_i1_const_ptr.indirect_del();
  // This calls *ptr_, but in a free function.
  // TODO(chandlerc): The I1_Class requirement below may be necessary, but is
  // not being added for the correct reasons even if so.
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: operator== is...*badinc-i1.h
  // IWYU: I1_const_ptr is...*badinc-i1.h
  (void)(local_i1_const_ptr == i1_class);
  // TODO(chandlerc): The I1_Class requirement below may be necessary, but is
  // not being added for the correct reasons even if so.
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: operator== is...*badinc-i1.h
  // IWYU: I1_const_ptr is...*badinc-i1.h
  (void)(i1_class == local_i1_const_ptr);
  // Also check the default (implicit) operator=
  // IWYU: I1_Class needs a declaration
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: I1_const_ptr is...*badinc-i1.h
  I1_const_ptr<I1_Class> local_i1_const_ptr2(NULL);
  // IWYU: I1_const_ptr is...*badinc-i1.h
  local_i1_const_ptr2 = local_i1_const_ptr;

  // We need the full I1_Class definition for the destructor.
  // IWYU: I1_Class needs a declaration
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: I1_const_ptr is...*badinc-i1.h
  (void)I1_const_ptr<I1_Class>(NULL);
  // IWYU: I1_Class needs a declaration
  // IWYU: I1_const_ptr is...*badinc-i1.h
  I1_const_ptr<I1_Class*> local_i1_const_ptrptr(NULL);
  // TODO(csilvers): IWYU: I1_Class needs a declaration
  // IWYU: I1_const_ptr is...*badinc-i1.h
  (void)(*local_i1_const_ptrptr);
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: I1_const_ptr is...*badinc-i1.h
  local_i1_const_ptrptr.deref_a();
  // This calls through to deref_a.
  // IWYU: I1_const_ptr is...*badinc-i1.h
  // IWYU: I1_Class is...*badinc-i1.h
  (void)(~local_i1_const_ptrptr);

  // IWYU: std::vector is...*<vector>
  // IWYU: I2_Enum is...*badinc-i2.h
  std::vector<I2_Enum> local_enum_vector;
  // I2_Enum here is redundant but harmless.
  // IWYU: I2_Enum is...*badinc-i2.h
  // IWYU: std::vector is...*<vector>
  // IWYU: I21 is...*badinc-i2.h
  local_enum_vector.push_back(I21);
  // IWYU: std::vector is...*<vector>
  // IWYU: I2_Enum is...*badinc-i2.h
  // IWYU: std::vector<.*>::iterator is...*<vector>
  for (std::vector<I2_Enum>::iterator it = local_enum_vector.begin();
       // IWYU: std::vector is...*<vector>
       // IWYU: std::vector<.*>::iterator is...*<vector>
       it != local_enum_vector.end(); ++it) {
    // IWYU: I2_Enum is...*badinc-i2.h
    // IWYU: std::vector is...*<vector>
    std::find(local_enum_vector.begin(), local_enum_vector.end(), *it);
  }
  // string also uses __normal_iterator.  Let's also verify that
  // adding two strings doesn't bring in a requirement for <memory>
  std::string s = std::string("aaa") + "bbb";
  for (std::string::iterator it = s.begin(); it != s.end(); ++it) {
    std::find(s.begin(), s.end(), *it);
  }
  for (Cc_string::iterator it = local_cc_string.begin();
       it != local_cc_string.end(); ++it) {
    std::find(local_cc_string.begin(), local_cc_string.end(), *it);
  }

  // This should result in an iwyu to #include <list>, not
  // forward-declare it, as STL types cannot be forward declared.
  // IWYU: std::list is...*<list>
  std::list<int>* list_ptr;
  list_ptr = 0;

  // Make sure we only report an iwyu for <new> because of placement-new.
  // We also need to check the argument to new.
  int* newed_int = new int;
  // IWYU: operator new is...*<new>
  new(newed_int) int(4);
  // IWYU: std::vector is...*<vector>
  // IWYU: I2_Enum is...*badinc-i2.h
  std::vector<I2_Enum>* newed_vector
      // IWYU: std::vector is...*<vector>
      // IWYU: I2_Enum is...*badinc-i2.h
      = new std::vector<I2_Enum>;
  // IWYU: i1_i1_classptr is...*badinc-i1.h
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: kI1ConstInt is...*badinc-i1.h
  // IWYU: operator new is...*<new>
  new (i1_i1_classptr) I1_Class(kI1ConstInt);
  // IWYU: I1_Class needs a declaration
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: kI1ConstInt is...*badinc-i1.h
  I1_Class* newed_i1_class_array = new I1_Class[kI1ConstInt];
  delete newed_int;
  delete (((newed_int)));
  // TODO(csilvers): IWYU: I2_Enum is...*badinc-i2.h
  // IWYU: std::vector is...*<vector>
  delete newed_vector;
  // IWYU: I1_Class is...*badinc-i1.h
  delete[] newed_i1_class_array;
  // IWYU: I1_Class is...*badinc-i1.h
  delete[] ((((newed_i1_class_array))));
  // IWYU: I1_Base needs a declaration
  // IWYU: I1_Base is...*badinc-i1.h
  // IWYU: I1_Class is...*badinc-i1.h
  delete[] static_cast<I1_Base*>(newed_i1_class_array);

  // Make sure constructor is analyzed when we new, and destructor is
  // analyzed when we delete.
  // IWYU: I1_TemplateClass is...*badinc-i1.h
  // IWYU: I2_Class needs a declaration
  // IWYU: I1_Struct needs a declaration
  I1_TemplateClass<I2_Class, I1_Struct>* newed_i1_template_class
      // IWYU: I1_Struct needs a declaration
      // IWYU: I1_Struct is...*badinc-i1.h
      // IWYU: I1_TemplateClass is...*badinc-i1.h
      // IWYU: I2_Class needs a declaration
      // IWYU: I2_Class is...*badinc-i2.h
      = new I1_TemplateClass<I2_Class, I1_Struct>;
  // IWYU: I1_TemplateClass is...*badinc-i1.h
  // IWYU: I2_Class needs a declaration
  // IWYU: I1_Struct needs a declaration
  I1_TemplateClass<I2_Class, I1_Struct>* newed_i1_template_class_array
      // IWYU: I1_Struct needs a declaration
      // IWYU: I1_Struct is...*badinc-i1.h
      // IWYU: I1_TemplateClass is...*badinc-i1.h
      // IWYU: I2_Class needs a declaration
      // IWYU: I2_Class is...*badinc-i2.h
      // IWYU: kI1ConstInt is...*badinc-i1.h
      = new I1_TemplateClass<I2_Class, I1_Struct>[kI1ConstInt];
  // IWYU: I1_TemplateClass is...*badinc-i1.h
  // IWYU: I2_Class needs a declaration
  // IWYU: I1_Struct needs a declaration
  I1_TemplateClass<I2_Class, I1_Struct>* newed_i1_template_class_ctor
      // IWYU: I1_Struct needs a declaration
      // IWYU: I1_TemplateClass is...*badinc-i1.h
      // IWYU: I2_Class needs a declaration
      // IWYU: I2_Class is...*badinc-i2.h
      = new I1_TemplateClass<I2_Class, I1_Struct>(i1_union);
  // IWYU: I1_TemplateClass is...*badinc-i1.h
  // IWYU: I2_Class needs a declaration
  // IWYU: I2_Class is...*badinc-i2.h
  // IWYU: I1_Struct needs a declaration
  char i1_templateclass_storage[sizeof(I1_TemplateClass<I2_Class, I1_Struct>)];
  // We need full type info for i1_templateclass because we never
  // fwd-declare a class with default template parameters.
  // IWYU: I1_TemplateClass is...*badinc-i1.h
  // IWYU: I2_Class needs a declaration
  // IWYU: I1_Struct needs a declaration
  I1_TemplateClass<I2_Class, I1_Struct>* placement_newed_i1_template_class
      // IWYU: I1_Struct needs a declaration
      // IWYU: I1_Struct is...*badinc-i1.h
      // IWYU: I1_TemplateClass is...*badinc-i1.h
      // IWYU: I2_Class needs a declaration
      // IWYU: I2_Class is...*badinc-i2.h
      // IWYU: operator new is...*<new>
      = new(i1_templateclass_storage) I1_TemplateClass<I2_Class, I1_Struct>();
  // IWYU: I1_Class needs a declaration
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: i1_ns::I1_NamespaceClass is...*badinc-i1.h
  I1_Class* i1_class_tpl_ctor = new I1_Class(&i1_namespace_class, 1);

  // TODO(csilvers): IWYU: I2_Class needs a declaration
  // IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
  // IWYU: I1_Struct is...*badinc-i1.h
  // IWYU: I1_TemplateClass is...*badinc-i1.h
  delete newed_i1_template_class;
  // TODO(csilvers): IWYU: I2_Class needs a declaration
  // IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
  // IWYU: I1_Struct is...*badinc-i1.h
  // IWYU: I1_TemplateClass is...*badinc-i1.h
  delete[] newed_i1_template_class_array;
  // TODO(csilvers): IWYU: I2_Class needs a declaration
  // IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
  // IWYU: I1_Struct is...*badinc-i1.h
  // IWYU: I1_TemplateClass is...*badinc-i1.h
  delete newed_i1_template_class_ctor;
  // Make sure we handle it right when we explicitly call the dtor, as well.
  // IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
  // IWYU: I1_TemplateClass is...*badinc-i1.h
  // IWYU: I1_Struct is...*badinc-i1.h
  placement_newed_i1_template_class->~I1_TemplateClass();
  // IWYU: I1_Class is...*badinc-i1.h
  delete i1_class_tpl_ctor;
  // Check that we discover constructor/destructor locations as well.
  // IWYU: I2_Class is...*badinc-i2.h
  // IWYU: I2_Class::I2_Class is...*badinc-i2-inl.h
  local_i2_class_ptr = new I2_Class("inl.h constructor");
  // IWYU: I2_Class is...*badinc-i2.h
  // IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
  delete local_i2_class_ptr;
  // Make sure we do this check even when it's via a template parameter.
  // (We need full type info for i1_templateclass because we never
  // fwd-declare a class with default template parameters.)
  // IWYU: I1_TemplateClass is...*badinc-i1.h
  // IWYU: I1_Enum is...*badinc-i1.h
  // IWYU: I2_Class needs a declaration
  I1_TemplateClass<I1_Enum, I2_Class>* local_i1_template_class_for_inl = NULL;
  // IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
  // IWYU: I1_TemplateClass is...*badinc-i1.h
  // IWYU: I2_Class is...*badinc-i2.h
  local_i1_template_class_for_inl->new_delete_bar();
  // Test per-class operator new
  // IWYU: I1_Struct needs a declaration
  // IWYU: I2_Struct needs a declaration
  CC_TemplateClass<I1_Struct, I2_Struct>* cc_template_class =
      // IWYU: I1_Struct needs a declaration
      // IWYU: I2_Struct needs a declaration
      // IWYU: I2_Struct is...*badinc-i2.h
      new CC_TemplateClass<I1_Struct,I2_Struct>;
  // IWYU: I1_Struct is...*badinc-i1.h
  delete cc_template_class;

  // While we're at it, make sure we get the same thing when creating
  // a local variable and a temporary, as we do when calling new/delete.
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: i1_ns::I1_NamespaceClass is...*badinc-i1.h
  I1_Class i1_class_tpl_ctor_local(&i1_namespace_class, 1);
  // We need the full type of I1_NamespaceClass because the ctor is a template.
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: i1_ns::I1_NamespaceClass is...*badinc-i1.h
  (void)I1_Class(&i1_namespace_class, 1);
  // IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
  // IWYU: I1_Struct needs a declaration
  // IWYU: I1_Struct is...*badinc-i1.h
  // IWYU: I1_TemplateClass is...*badinc-i1.h
  // IWYU: I2_Class needs a declaration
  // IWYU: I2_Class is...*badinc-i2.h
  I1_TemplateClass<I2_Class, I1_Struct> local_i1_templateclass(i1_union);
  // IWYU: I2_Class::~I2_Class is...*badinc-i2-inl.h
  // IWYU: I1_Struct needs a declaration
  // IWYU: I1_Struct is...*badinc-i1.h
  // IWYU: I1_TemplateClass is...*badinc-i1.h
  // IWYU: I2_Class needs a declaration
  // IWYU: I2_Class is...*badinc-i2.h
  (void)I1_TemplateClass<I2_Class, I1_Struct>(i1_union);

  (void)(*(new int(4)));
  (void)((new int[10])[0]);
  // Make sure we don't claim i2_newed and i2_deleted are "ptr only".
  // IWYU: I2_ThisClassIsOnlyNewed needs a declaration
  I2_ThisClassIsOnlyNewed* i2_newed = 0;
  // IWYU: I2_ThisClassIsOnlyDeleted needs a declaration
  I2_ThisClassIsOnlyDeleted* i2_deleted = 0;
  // IWYU: I2_ThisClassIsOnlyDeletedPtr needs a declaration
  I2_ThisClassIsOnlyDeletedPtr** i2_deleted_ptr = 0;
  // IWYU: I2_ThisClassIsOnlyNewed is...*badinc-i2.h
  i2_newed = new I2_ThisClassIsOnlyNewed;
  // IWYU: I2_ThisClassIsOnlyDeleted is...*badinc-i2.h
  delete i2_deleted;
  delete i2_deleted_ptr;
  delete i2_deleted_ptr;  // make sure each delete doesn't eat up a star

  // Make sure we don't claim set::iterator is defined in map.
  // IWYU: std::set is...*<set>
  std::set<int> localset;
  // IWYU: std::set is...*<set>
  // IWYU: std::set<.*>::iterator is...*<set>
  std::set<int>::iterator it_set = localset.begin();

  // Lots of weird stuff can happen with iterators, especially regarding const.
  // IWYU: std::vector is...*<vector>
  std::vector<float> float_vector;
  // IWYU: std::vector is...*<vector>
  // IWYU: std::vector<.*>::const_iterator is...*<vector>
  std::vector<float>::const_iterator float_it = float_vector.begin();
  // IWYU: std::vector is...*<vector>
  // IWYU: std::vector<.*>::const_iterator is...*<vector>
  const std::vector<float>::const_iterator float_constit = float_vector.begin();
  // IWYU: std::vector is...*<vector>
  (void)(float_it == float_constit);
  // IWYU: std::vector is...*<vector>
  // IWYU: std::vector<.*>::const_iterator is...*<vector>
  std::vector<float>::const_iterator float_forit;
  // IWYU: std::vector is...*<vector>
  // IWYU: std::vector<.*>::const_iterator is...*<vector>
  for (float_forit = float_vector.begin(); ;) ;
  // IWYU: std::vector is...*<vector>
  // IWYU: std::vector<.*>::const_iterator is...*<vector>
  for (std::vector<float>::const_iterator it = float_vector.begin(); ;) ;
  // IWYU: std::vector is...*<vector>
  // IWYU: std::vector<.*>::const_iterator is...*<vector>
  for (const std::vector<float>::const_iterator it = float_vector.begin(); ;) ;
  // We special-case vector<>::iterator.  Make sure it holds for
  // reverse_iterator too.
  // IWYU: std::vector is...*<vector>
  // IWYU: std::vector<.*>::reverse_iterator is...*<vector>
  for (std::vector<float>::reverse_iterator
           // IWYU: std::vector is...*<vector>
           float_reverse_it = float_vector.rbegin();
       // IWYU: std::vector is...*<vector>
       float_reverse_it != float_vector.rbegin();
       // IWYU: std::vector is...*<vector>
       // IWYU: std::vector<.*>::reverse_iterator is...*<vector>
       ++float_reverse_it) ;
  // IWYU: std::vector is...*<vector>
  // IWYU: std::vector<.*>::const_reverse_iterator is...*<vector>
  for (std::vector<float>::const_reverse_iterator
           // We need const_reverse_iterator here because of the
           // conversion from reverse_iterator (from rbegin()).
           // IWYU: std::vector<.*>::const_reverse_iterator is...*<vector>
           // IWYU: std::vector is...*<vector>
           float_const_reverse_it = float_vector.rbegin();
       // IWYU: std::vector is...*<vector>
       float_const_reverse_it != float_vector.rend();
       // IWYU: std::vector is...*<vector>
       // IWYU: std::vector<.*>::const_reverse_iterator is...*<vector>
       ++float_const_reverse_it) ;

  // Also test while and if initializers.
  // IWYU: I1_Class needs a declaration
  while (I1_Class* i = NULL) {
  }
  // IWYU: I1_Class needs a declaration
  if (I1_Class* i = NULL) i = NULL;

  // Test some macros, including one we shouldn't mark as in-use.
  // IWYU: MACRO_CALLING_I2_FUNCTION is...*badinc-i2.h
  MACRO_CALLING_I2_FUNCTION;
  // Here are some uses of UNUSED_MACRO that we should not find:
  /*
    UNUSED_MACRO
  */
  (void)("#include \\\"UNUSED_MACRO\"");

  // Test if and while.
  // IWYU: I1_Function is...*badinc-i1.h
  if (I1_Function(0)) {        // returns an I1Enum
    // IWYU: I2_Struct is...*badinc-i2.h
    // IWYU: I2_Function is...*badinc-i2.h
    while (I2_Function(0)) ;   // returns a I2_Struct, converted to a bool
  }

  // Test calling virtual methods.
  // IWYU: I2_Subclass needs a declaration
  I2_Subclass* i2_subclass = 0;
  // IWYU: I2_Subclass is...*badinc-i2.h
  i2_subclass->Impl();
  // IWYU: I2_Subclass is...*badinc-i2.h
  i2_subclass->Abstract();
  // IWYU: I2_Subclass is...*badinc-i2.h
  i2_subclass->BaseOnly();
  // IWYU: I2_Subclass is...*badinc-i2.h
  i2_subclass->NonvirtualBaseOnly();

  // Test templatized functions.
  H_TemplateFunction(1);
  // IWYU: I11 is...*badinc-i1.h
  // IWYU: I1_Enum is...*badinc-i1.h
  H_TemplateFunction(I11);
  // IWYU: I11 is...*badinc-i1.h
  H_TemplateFunction<int>(I11);
  // IWYU: I1_Class needs a declaration
  H_TemplateFunction<I1_Class*>(&i1_class);
  H_TemplateFunction(&i1_class);
  // IWYU: I1_Enum is...*badinc-i1.h
  // IWYU: I22 is...*badinc-i2.h
  h_templateclass2.static_out_of_line(I22);
  // IWYU: I22 is...*badinc-i2.h
  h_templateclass2.h_nested_struct.tplnested(I22);
  // IWYU: I22 is...*badinc-i2.h
  h_templateclass2.h_nested_struct.static_tplnested(I22);
  // This should not cause warnings for the i2_class destructor
  h_templateclass2.uses_i2class();
  // IWYU: I1_TemplateFunction is...*badinc-i1.h
  I1_TemplateFunction(i1_class_ptr);
  // IWYU: I1_TemplateFunction is...*badinc-i1.h
  // IWYU: I1_Class needs a declaration
  I1_TemplateFunction<I1_Class*>(i1_class_ptr);
  // Try again, but with a typedef
  Cc_typedef cc_typedef;
  // TODO(csilvers): figure out the template arg here is really a
  //    typedef (tricky because we need to call the I1_Class ctor),
  //    and don't add it to tpl-types-of-interest.
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: I1_TemplateFunction is...*badinc-i1.h
  I1_TemplateFunction(cc_typedef);
  // IWYU: I1_TemplateFunction is...*badinc-i1.h
  I1_TemplateFunction<Cc_typedef>(cc_typedef);
  // IWYU: I1_TemplateFunction is...*badinc-i1.h
  I1_TemplateFunction<Cc_typedef>(i1_class);

  // IWYU: I1_Class is...*badinc-i1.h
  i1_class.I1_ClassTemplateFunction(&i1_struct);
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: I1_Struct needs a declaration
  i1_class_ptr->I1_ClassTemplateFunction<I1_Struct*>(&i1_struct);
  // IWYU: I1_Class is...*badinc-i1.h
  I1_Class::I1_StaticClassTemplateFunction(&i1_struct);
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: I1_Struct needs a declaration
  I1_Class::I1_StaticClassTemplateFunction<I1_Struct*>(&i1_struct);

  // Test default (compiler-defined) copy constructor/operator=/operator==
  // IWYU: I11 is...*badinc-i1.h
  D1_CopyClass local_d1_copy_class(D1CopyClassFn(I11));
  // IWYU: I12 is...*badinc-i1.h
  local_d1_copy_class = D1CopyClassFn(I12);
  local_d1_copy_class.a();

  // Test (templated) function pointers and method pointers.
  // IWYU: I1_Class needs a declaration
  // IWYU: I1_Enum is...*badinc-i1.h
  // IWYU: I1_Function is...*badinc-i1.h
  I1_Enum (*local_fn_ptr)(I1_Class*) = &I1_Function;
  // IWYU: I1_Class is...*badinc-i1.h
  int (*static_method_ptr)() = &I1_Class::s;
  // IWYU: I1_Struct is...*badinc-i1.h
  // IWYU: I1_Struct needs a declaration
  // IWYU: I1_TemplateMethodOnlyClass is...*badinc-i1.h
  int (*tpl_fn_ptr)() = &I1_TemplateMethodOnlyClass<I1_Struct>::stat;
  // IWYU: I1_Class is...*badinc-i1.h
  // IWYU: I1_Class needs a declaration
  // IWYU: I1_Struct needs a declaration
  // IWYU: I1_TemplateMethodOnlyClass is...*badinc-i1.h
  int (*tpl_fn_ptr2)() = &I1_TemplateMethodOnlyClass<I1_Struct>::t<I1_Class>;
  // IWYU: I1_Class is...*badinc-i1.h
  int (I1_Class::*method_ptr)() const = &I1_Class::a;
  // IWYU: I1_Struct is...*badinc-i1.h
  // IWYU: I1_Struct needs a declaration
  // IWYU: I1_TemplateMethodOnlyClass is...*badinc-i1.h
  I1_Struct (I1_TemplateMethodOnlyClass<I1_Struct>::*tpl_method_ptr)()
      // IWYU: I1_Struct is...*badinc-i1.h
      // IWYU: I1_Struct needs a declaration
      // IWYU: I1_TemplateMethodOnlyClass is...*badinc-i1.h
      = &I1_TemplateMethodOnlyClass<I1_Struct>::a;
  // IWYU: I1_Struct needs a declaration
  // IWYU: I1_TemplateMethodOnlyClass is...*badinc-i1.h
  // IWYU: I2_Struct is...*badinc-i2.h
  I2_Struct (I1_TemplateMethodOnlyClass<I1_Struct>::*tpl_method_ptr2)()
      // IWYU: I1_Struct needs a declaration
      // IWYU: I2_Struct is...*badinc-i2.h
      // IWYU: I2_Struct needs a declaration
      // IWYU: I1_TemplateMethodOnlyClass is...*badinc-i1.h
      = &I1_TemplateMethodOnlyClass<I1_Struct>::c<I2_Struct>;

  // Check use of a macro inside an #ifdef.
  // IWYU: I2_MACRO is...*badinc-i2.h
#ifdef I2_MACRO
  // IWYU: kI1ConstInt is...*badinc-i1.h
  int i2_macro_var = kI1ConstInt;
#endif

  // IWYU: rand is...*<stdlib.h>
  return rand();           // use a function from stdlib.h
}

// TODO(csilvers): the delete of <locale> should be line 56, not 35
// Some notes:
// * We do not need to #include badinc-i2.h, badinc_i2-inl.h, set,
//   stdio.h, or vector, because badinc.h is adding them for us.
// * We *do* need to #include ctype.h, even though badinc.h #includes
//   it, because badinc.h is removing that dependency.
// * We're removing <algorithm> twice because it's in the file 3 times.
/**** IWYU_SUMMARY

tests/cxx/badinc.cc should add these lines:
#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <list>
#include <new>
#include "tests/cxx/badinc-i1.h"
class D2_Class;
class D2_ForwardDeclareClass;
class D2_Subclass;
class I1_ForwardDeclareClass;
namespace d3_namespace { struct D3_Struct; }
namespace i3_ns1 { namespace i3_ns2 { namespace i3_ns3 { template <typename A, int B> struct I3_ForwardDeclareNamespaceTemplateStruct; } } }
namespace i3_ns1 { namespace { struct I3_UnnamedNamespaceStruct; } }
struct I3_ForwardDeclareStruct;
template <typename A, int B, char C> struct I3_ForwardDeclareTemplateStruct;

tests/cxx/badinc.cc should remove these lines:
- #include <math.h>  // lines XX-XX
- #include <algorithm>  // lines XX-XX
- #include <algorithm>  // lines XX-XX
- #include <cwchar>  // lines XX-XX
- #include <locale>  // lines XX-XX
- #include "tests/cxx/badinc-d2.h"  // lines XX-XX
- class Cc_ForwardDeclare_Function::I2_Class;  // lines XX-XX
- class I3_UnusedClass;  // lines XX-XX
- template <class T = I1_Class, I1_Enum E = I11> class Cc_DeclareOnlyTemplateClass;  // lines XX-XX

The full include-list for tests/cxx/badinc.cc:
#include "tests/cxx/badinc.h"
#include "tests/cxx/badinc-inl.h"
#include <ctype.h>  // for isascii
#include <setjmp.h>
#include <stdarg.h>  // for va_list
#include <stddef.h>  // for offsetof, size_t
#include <stdlib.h>  // for rand
#include <algorithm>  // for find
#include <fstream>  // for fstream
#include <list>  // for list
#include <new>  // for operator new
#include <string>  // for allocator, basic_string, basic_string<>::iterator, char_traits, operator+, string
#include <typeinfo>  // for type_info
#include "tests/cxx/badinc-d1.h"  // for D1CopyClassFn, D1Function, D1_Class, D1_CopyClass, D1_Enum, D1_Enum::D11, D1_I1_Typedef, D1_StructPtr, D1_Subclass, D1_TemplateClass, D1_TemplateStructWithDefaultParam, MACRO_CALLING_I4_FUNCTION
#include "tests/cxx/badinc-d4.h"  // for D4_ClassForOperator, operator<<
#include "tests/cxx/badinc-i1.h"  // for EmptyDestructorClass, H_Class::H_Class_DefinedInI1, I1_And_I2_OverloadedFunction, I1_Base, I1_Class, I1_Class::NestedStruct, I1_ClassPtr, I1_Enum, I1_Enum::I11, I1_Enum::I12, I1_Enum::I13, I1_Function, I1_FunctionPtr, I1_I2_Class_Typedef, I1_MACRO_LOGGING_CLASS, I1_MACRO_SYMBOL_WITHOUT_VALUE, I1_MACRO_SYMBOL_WITH_VALUE, I1_MACRO_SYMBOL_WITH_VALUE0, I1_MACRO_SYMBOL_WITH_VALUE2, I1_ManyPtrStruct (ptr only), I1_MemberPtr, I1_NamespaceClass, I1_NamespaceStruct, I1_NamespaceTemplateFn, I1_OverloadedFunction, I1_PtrAndUseOnSameLine, I1_PtrDereferenceClass, I1_PtrDereferenceStatic, I1_PtrDereferenceStruct, I1_SiblingClass, I1_StaticMethod, I1_Struct, I1_Subclass, I1_SubclassesI2Class, I1_TemplateClass, I1_TemplateClass<>::I1_TemplateClass_int, I1_TemplateClassFwdDeclaredInD2 (ptr only), I1_TemplateFunction, I1_TemplateMethodOnlyClass, I1_TemplateSubclass, I1_Typedef, I1_TypedefOnly_Class, I1_TypedefOnly_Class<>::i, I1_Union, I1_UnnamedStruct, I1_UnusedNamespaceStruct (ptr only), I1_const_ptr, I2_OperatorDefinedInI1Class::operator<<, MACRO_CALLING_I6_FUNCTION, OperateOn, i1_GlobalFunction, i1_i1_classptr, i1_int, i1_int_global, i1_int_global2, i1_int_global2sub, i1_int_global3, i1_int_global3sub, i1_int_global4, i1_int_global4sub, i1_int_globalsub, i1_ns4, kI1ConstInt, operator==
#include "tests/cxx/badinc2.c"
class D2_Class;
class D2_ForwardDeclareClass;
class D2_Subclass;
class ForwardDeclareOnlyClass;  // lines XX-XX
class ForwardDeclareOnlyForTypedefClass;  // lines XX-XX
class I1_ForwardDeclareClass;
class I3_ForwardDeclareClass;  // lines XX-XX
class MacroClass;  // lines XX-XX
namespace d3_namespace { struct D3_Struct; }
namespace i3_ns1 { namespace i3_ns2 { namespace i3_ns3 { struct I3_ForwardDeclareNamespaceStruct; } } }  // lines XX-XX
namespace i3_ns1 { namespace i3_ns2 { namespace i3_ns3 { template <typename A, int B> struct I3_ForwardDeclareNamespaceTemplateStruct; } } }
namespace i3_ns1 { namespace { struct I3_UnnamedNamespaceStruct; } }
struct Cc_C_Struct;  // lines XX-XX
struct I3_ForwardDeclareStruct;
template <class T> struct Cc_OnlySpecializedStruct;  // lines XX-XX
template <typename A, int B, char C> struct I3_ForwardDeclareTemplateStruct;
template <typename T> struct I3_SimpleForwardDeclareTemplateStruct;  // lines XX-XX+1

***** IWYU_SUMMARY */

// TODO(csilvers): some new tests to add:
//   29) .h fwd-declares a class and .cc does as well.  Do we attribute to .cc?
//   30) forward-declare a class while inside a namespace.
//   31) "using foo::bar; Baz* x;" forward declare foo::bar::Baz properly?
//   32) "namespace b = foo::bar; b::Baz* x; forward declare Baz properly?
//   34) no mystruct fwd-decl for 'struct mystruct { mystruct* next; } myvar;'
//   41) template<class T, class V=hash<T> > void fn(), call fn<OperateOn>
//   43) *foo, where foo is a typedef to a pointer to an iwyu type.
//   44) typedef Foo Bar[sizeof(Baz)] -- make sure it says we need full type
//       info for Baz.
//   45) Define a global operator-new in badinc-i1.h and call it from badinc.cc
