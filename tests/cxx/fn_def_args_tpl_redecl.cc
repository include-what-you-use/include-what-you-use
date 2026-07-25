//===--- fn_def_args_tpl_redecl.cc - iwyu test ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

#include "tests/cxx/fn_def_args_tpl_redecl.h"

int main() {
  // IWYU: TemplateArgument needs a declaration
  // IWYU: TemplateArgument is...*fn_def_args_tpl_redecl-i3.h
  FnTplDeclOnly<TemplateArgument>();
}

/**** IWYU_SUMMARY

tests/cxx/fn_def_args_tpl_redecl.cc should add these lines:
#include "tests/cxx/fn_def_args_tpl_redecl-i3.h"

tests/cxx/fn_def_args_tpl_redecl.cc should remove these lines:

The full include-list for tests/cxx/fn_def_args_tpl_redecl.cc:
#include "tests/cxx/fn_def_args_tpl_redecl.h"
#include "tests/cxx/fn_def_args_tpl_redecl-i3.h"  // for TemplateArgument

***** IWYU_SUMMARY */
