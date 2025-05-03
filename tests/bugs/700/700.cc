//===--- 700.cc - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include "header.h"

namespace BarNs {
  // IWYU: BarNs::FooClass needs a declaration
  void f(FooClass *);
}

// There are a few possible resolutions for this example:
//
// Two forward declarations:
//   namespace FooNs { class FooClass; }
//   namespace BarNs { using namespace FooNs; }
//
// or maybe a "fake" forward-declaration in BarNs:
//   namespace BarNs { class FooClass; }
//
// or keep header.h unchanged, because the dependency introduced by 'using
// namespace FooNs' is not worth replicating as a forward-decl.

/**** IWYU_SUMMARY

(tests/bugs/700/700.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
