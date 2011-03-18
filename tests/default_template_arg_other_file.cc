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

#include "tests/default_template_arg_other_file-d1.h"
#include "tests/default_template_arg_other_file-d2.h"

int main() {
  // TODO(csilvers): IWYU: OperateOn is...*default_template_arg_other_file-i2.h
  // IWYU: MyClass needs a declaration
  TemplateStruct<MyClass> ts;
  // IWYU: OperateOn is...*default_template_arg_other_file-i2.h
  ts.a();
}

/**** IWYU_SUMMARY

tests/default_template_arg_other_file.cc should add these lines:
#include "tests/default_template_arg_other_file-i2.h"

tests/default_template_arg_other_file.cc should remove these lines:
- #include "tests/default_template_arg_other_file-d2.h"  // lines XX-XX

The full include-list for tests/default_template_arg_other_file.cc:
#include "tests/default_template_arg_other_file-d1.h"  // for TemplateStruct
#include "tests/default_template_arg_other_file-i2.h"  // for MyClass (ptr only), OperateOn

***** IWYU_SUMMARY */
