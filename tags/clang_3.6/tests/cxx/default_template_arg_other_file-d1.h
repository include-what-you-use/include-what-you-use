//===--- default_template_arg_other_file-d1.h - test input file for iwyu --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/default_template_arg_other_file-o1.h"

// We provide one specialization here.
class IntendsToProvideClass { };
template<> class OperateOn<IntendsToProvideClass> { };

// And one partial specialization as well.
template<typename T> class OperateOn<T*> { };

template<typename T> class IntendsToProvideTplClass { };
template<typename T> class OperateOn<IntendsToProvideTplClass<T> > { };

template<class T, class Functor = OperateOn<T> > class TemplateStructHelper {
 public:
  void a() {
    Functor f;
    (void)f;
  }
};

// To make this example as much like hash_set<> as possible, the outer
// class is really just a container around the class that does work.
template<class T, class Functor = OperateOn<T> > class TemplateStruct {
 private:
  typedef TemplateStructHelper<T, Functor> _TS;
  _TS ts;
 public:
  void a() { return ts.a(); }
};
