//===--- implementation_file.cc - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -Xiwyu --check_also=tests/cxx/implementation_file-d2.h \
//            -Xiwyu --mapping_file=tests/cxx/implementation_file.imp

// Tests that IWYU detects implementation files, with no mapping or pragma.
// implementation_file.h keeps its inline definitions in
// implementation_file-impl.icc and #includes that file itself.  IWYU
// recognizes it because it defines what implementation_file.h declares.
// So it is analyzed with implementation_file.h, may use what that file
// #includes, and is private to it.
//
// implementation_file-impl.icc itself #includes
// implementation_file-detail.icc, which defines more of what
// implementation_file.h declares: it is an implementation file of
// implementation_file.h too.
//
// The same holds for the implementation files of other headers.
// implementation_file-d1.h is not reported on, but users of what its
// implementation file defines are still told to #include it, not the
// implementation file.  implementation_file-d2.h is reported on thanks to
// --check_also, and so is its implementation file, which already has a
// visibility.  A mapping file gives the implementation file of
// implementation_file-d3.h a visibility too: users of what it defines
// are told to #include implementation_file-d3-all.h.

// Makes clang look up implementation_file-impl.icc before
// implementation_file.h, so that IWYU sees it first.
#if __has_include("tests/cxx/implementation_file-impl.icc")
#endif

#include "tests/cxx/implementation_file-d1.h"
#include "tests/cxx/implementation_file-d2.h"
#include "tests/cxx/implementation_file-d3.h"
#include "tests/cxx/implementation_file.h"

void Use() {
  ImplFileClass c;
  c.InlineMethod();
  c.DetailMethod();
  c.TemplateMethod(1);
  ImplFileFunction(1);
  ImplFileTemplate(1);

  ImplFileD1Class d1;
  d1.InlineMethod();

  ImplFileD2Class d2;
  d2.InlineMethod();

  ImplFileD3Class d3;
  // IWYU: ImplFileD3Class::InlineMethod() is...*implementation_file-d3-all.h
  d3.InlineMethod();
}

/**** IWYU_SUMMARY

tests/cxx/implementation_file.cc should add these lines:
#include "tests/cxx/implementation_file-d3-all.h"

tests/cxx/implementation_file.cc should remove these lines:

The full include-list for tests/cxx/implementation_file.cc:
#include "tests/cxx/implementation_file.h"
#include "tests/cxx/implementation_file-d1.h"  // for ImplFileD1Class
#include "tests/cxx/implementation_file-d2.h"  // for ImplFileD2Class
#include "tests/cxx/implementation_file-d3-all.h"  // for ImplFileD3Class::InlineMethod
#include "tests/cxx/implementation_file-d3.h"  // for ImplFileD3Class

***** IWYU_SUMMARY */
