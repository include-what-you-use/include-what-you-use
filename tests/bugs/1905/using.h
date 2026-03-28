//===--- using.h - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

class Ret;
class Arg;

namespace ns {
  using ::Ret;
  using ::Arg;
}

extern Ret r;
extern Arg a;

/**** IWYU_SUMMARY

(tests/bugs/1905/using.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
