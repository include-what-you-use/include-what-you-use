//===--- default_template_arg_other_file.cc - test input file for iwyu ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests the equivalent of doing
//    hash_set<MyClass> foo;
// where hash<MyClass> is defined in some .h file.  This should
// require a needs-to-include dependency on that .h file, to get
// its specialization of hash<>.

#include "tests/cxx/default_template_arg_other_file-d1.h"
#include "tests/cxx/default_template_arg_other_file-d2.h"

int main() {
  // TODO(csilvers): IWYU: OperateOn is...*default_template_arg_other_file-i2.h
  // IWYU: MyClass needs a declaration
  TemplateStruct<MyClass> ts;
  // IWYU: OperateOn is...*default_template_arg_other_file-i2.h
  ts.a();

  // TODO(csilvers): IWYU: OperateOn is...*default_template_arg_other_file-i2.h
  // IWYU: TplClass needs a declaration
  // IWYU: MyClass needs a declaration
  TemplateStruct<TplClass<MyClass> > ts2;
  // IWYU: OperateOn is...*default_template_arg_other_file-i2.h
  ts2.a();

  // Make sure that we don't require OperateOn when we don't have a
  // type with an explicit or partial specialization.
  TemplateStruct<int> ts3;
  ts3.a();

  // We also shouldn't require OperateOn when we have an explicit
  // specialization in the same file as the main definition.
  // (Since then the provider of OperateOn "intends to provide"
  // the specialization.)
  TemplateStruct<IntendsToProvideClass> ts4;
  ts4.a();
  TemplateStruct<IntendsToProvideTplClass<IntendsToProvideClass> > ts5;
  ts5.a();

  // Same when it's a partial specialization.
  // IWYU: MyClass needs a declaration
  TemplateStruct<MyClass*> ts6;
  ts6.a();
  // IWYU: MyClass needs a declaration
  // IWYU: TplClass needs a declaration
  TemplateStruct<TplClass<MyClass>*> ts7;
  ts7.a();
}

/**** IWYU_SUMMARY

tests/cxx/default_template_arg_other_file.cc should add these lines:
#include "tests/cxx/default_template_arg_other_file-i2.h"

tests/cxx/default_template_arg_other_file.cc should remove these lines:
- #include "tests/cxx/default_template_arg_other_file-d2.h"  // lines XX-XX

The full include-list for tests/cxx/default_template_arg_other_file.cc:
#include "tests/cxx/default_template_arg_other_file-d1.h"  // for IntendsToProvideClass (ptr only), IntendsToProvideTplClass (ptr only), TemplateStruct
#include "tests/cxx/default_template_arg_other_file-i2.h"  // for MyClass (ptr only), OperateOn, TplClass (ptr only)

***** IWYU_SUMMARY */
