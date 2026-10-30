//===--- macro_use_before_decl.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// An X-macro defined before the class whose nested enum it fills in, and
// expanded both here and in macro_use_before_decl.cc.

#define FOR_EACH_SHAPE(ACTION) \
  ACTION(Circle)               \
  ACTION(Square)

class Shapes {
 public:
  enum Shape : unsigned {
#define SHAPE_ENUMERATOR(S) S,
    FOR_EACH_SHAPE(SHAPE_ENUMERATOR)
#undef SHAPE_ENUMERATOR
  };

  static const Shape kShapes[];
};

/**** IWYU_SUMMARY

(tests/cxx/macro_use_before_decl.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
