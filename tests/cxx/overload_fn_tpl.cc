//===--- overload_fn_tpl.cc - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

#include "tests/cxx/overload_fn_tpl-d1.h"
#include "tests/cxx/overload_fn_tpl-d2.h"
#include "tests/cxx/overload_fn_tpl-d3.h"

// Test function template use with dependent types inside an uninstantiated
// template.
template <typename T>
void UserTemplate(T v) {
  // There are multiple overloads available from different files -i1.h and
  // -i2.h. An arbitrary decl is reported.
  // IWYU: ns::TplFn(:0, :1) is...*-i1.h
  ns::TplFn(v, bool{});

  // There's technically a better overload in -i2.h, but an arbitrary decl is
  // reported. User can include -i2.h directly to state a preference, see
  // TplFnDirect below.
  int i = 0;
  // IWYU: ns::TplFn(:0, :1) is...*-i1.h
  ns::TplFn(&v, &i);

  // When all overloads are in the same file, an arbitrary decl from that file
  // is reported to keep the file included.
  // IWYU: ns::TplFnSameFile(:0) is...*-i3.h
  ns::TplFnSameFile(v);

  // No diagnostic here. When decls are directly included, we assume the user
  // made an active choice for which overloads to make available. Both -d2.h and
  // -d3.h are preserved in the summary below.
  ns::TplFnDirect(v, i);

  // Trigger ADL. No diagnostic or tracking expected.
  Undefined(v);
}

/**** IWYU_SUMMARY

tests/cxx/overload_fn_tpl.cc should add these lines:
#include "tests/cxx/overload_fn_tpl-i1.h"
#include "tests/cxx/overload_fn_tpl-i3.h"

tests/cxx/overload_fn_tpl.cc should remove these lines:
- #include "tests/cxx/overload_fn_tpl-d1.h"  // lines XX-XX

The full include-list for tests/cxx/overload_fn_tpl.cc:
#include "tests/cxx/overload_fn_tpl-d2.h"  // for TplFnDirect
#include "tests/cxx/overload_fn_tpl-d3.h"  // for TplFnDirect
#include "tests/cxx/overload_fn_tpl-i1.h"  // for TplFn
#include "tests/cxx/overload_fn_tpl-i3.h"  // for TplFnSameFile

***** IWYU_SUMMARY */
