//===--- derived_function_tpl_args.cc - test input file for iwyu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests calls to a templated function, especially ones where the
// calls do not specify the template args explicitly, but instead
// have them derived from the function arguments (including return
// type).  clang desugars the template types in this case, and we
// make some effort to undo this work and get back to what the
// template args 'logically' are.

#include "tests/derived_function_tpl_args-d1.h"

template<typename T> void Fn(T t) {
  T dummy;
  (void)dummy;
}

template<typename T> void FnWithPtr(T* t) {
  T dummy;
  (void)dummy;
}

template<typename T> void FnWithReference(const T& t) {
  T dummy;
  (void)dummy;
}

template<typename T> struct TplClass {
  T t;
};

int main() {
  // IWYU: IndirectClass is...*derived_function_tpl_args-i1.h
  IndirectClass ic;
  // IWYU: IndirectClass needs a declaration
  IndirectClass* ic_ptr = 0;

  // IWYU: IndirectClass is...*derived_function_tpl_args-i1.h
  Fn(ic);
  Fn(ic_ptr);
  // IWYU: IndirectClass is...*derived_function_tpl_args-i1.h
  FnWithPtr(ic_ptr);
  // IWYU: IndirectClass is...*derived_function_tpl_args-i1.h
  FnWithReference(ic);
  FnWithReference(ic_ptr);

  // Do it again, to make sure that the template cache is working properly.
  // IWYU: IndirectClass is...*derived_function_tpl_args-i1.h
  Fn(ic);
  Fn(ic_ptr);
  // IWYU: IndirectClass is...*derived_function_tpl_args-i1.h
  FnWithPtr(ic_ptr);
  // IWYU: IndirectClass is...*derived_function_tpl_args-i1.h
  FnWithReference(ic);
  FnWithReference(ic_ptr);

  // Now try again, but with a typedef.
  // IWYU: IndirectClass is...*derived_function_tpl_args-i1.h
  typedef IndirectClass LocalClass;
  LocalClass lc;
  LocalClass* lc_ptr = 0;
  // TODO(csilvers): this is wrong. Figure out how to resugar in this case too.
  // IWYU: IndirectClass is...*derived_function_tpl_args-i1.h
  Fn(lc);
  Fn(lc_ptr);
  FnWithPtr(lc_ptr);
  // TODO(csilvers): this is wrong. Figure out how to resugar in this case too.
  // IWYU: IndirectClass is...*derived_function_tpl_args-i1.h
  FnWithReference(lc);
  FnWithReference(lc_ptr);

  // And try again, but with namespaces.  This makes sure we don't
  // get tripped up by ElaboratedType's.
  // IWYU: ns::NsClass is...*derived_function_tpl_args-i1.h
  ns::NsClass ns_ic;
  // IWYU: ns::NsClass needs a declaration
  ns::NsClass* ns_ic_ptr = 0;

  // IWYU: ns::NsClass is...*derived_function_tpl_args-i1.h
  Fn(ns_ic);
  Fn(ns_ic_ptr);
  // IWYU: ns::NsClass is...*derived_function_tpl_args-i1.h
  FnWithPtr(ns_ic_ptr);
  // IWYU: ns::NsClass is...*derived_function_tpl_args-i1.h
  FnWithReference(ns_ic);
  FnWithReference(ns_ic_ptr);

  // Handle the case where the sugaring is due to default template args.
  // IWYU: IndirectTplClass is...*derived_function_tpl_args-i1.h
  // IWYU: IndirectClass needs a declaration
  IndirectTplClass<IndirectClass> itc;
  // We need the full type for IndirectTplClass because it has a default arg.
  // IWYU: IndirectTplClass is...*derived_function_tpl_args-i1.h
  // IWYU: IndirectClass needs a declaration
  IndirectTplClass<IndirectClass>* itc_ptr;
  // IWYU: IndirectTplClass is...*derived_function_tpl_args-i1.h
  Fn(itc);
  Fn(itc_ptr);
  // IWYU: IndirectTplClass is...*derived_function_tpl_args-i1.h
  FnWithPtr(itc_ptr);
  // IWYU: IndirectTplClass is...*derived_function_tpl_args-i1.h
  FnWithReference(itc);
  FnWithReference(itc_ptr);

  // If we have the args explicitly, we're responsible for them.
  // IWYU: IndirectTplClass is...*derived_function_tpl_args-i1.h
  // IWYU: IndirectClass needs a declaration
  // IWYU: ns::NsClass needs a declaration
  IndirectTplClass<IndirectClass, ns::NsClass> itc2;
  // IWYU: IndirectTplClass is...*derived_function_tpl_args-i1.h
  // IWYU: IndirectClass needs a declaration
  // IWYU: ns::NsClass needs a declaration
  IndirectTplClass<IndirectClass, ns::NsClass>* itc2_ptr;
  // IWYU: IndirectTplClass is...*derived_function_tpl_args-i1.h
  Fn(itc2);
  Fn(itc2_ptr);
  // IWYU: IndirectTplClass is...*derived_function_tpl_args-i1.h
  FnWithPtr(itc2_ptr);
  // IWYU: IndirectTplClass is...*derived_function_tpl_args-i1.h
  FnWithReference(itc2);
  FnWithReference(itc2_ptr);

  // If we specify explicit template args, those should override everything.
  FnWithPtr<LocalClass>(ic_ptr);
  FnWithReference<LocalClass>(ic);
}

/**** IWYU_SUMMARY

tests/derived_function_tpl_args.cc should add these lines:
#include "tests/derived_function_tpl_args-i1.h"

tests/derived_function_tpl_args.cc should remove these lines:
- #include "tests/derived_function_tpl_args-d1.h"  // lines XX-XX

The full include-list for tests/derived_function_tpl_args.cc:
#include "tests/derived_function_tpl_args-i1.h"  // for IndirectClass, IndirectTplClass, NsClass

***** IWYU_SUMMARY */
