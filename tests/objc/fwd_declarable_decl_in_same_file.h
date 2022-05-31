//===--- fwd_declarable_decl_in_same_file.h - test input file for iwyu ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

@interface Foo {
}
@end

@interface Bar {
 @private
  Foo *foo;
}
@end

/**** IWYU_SUMMARY

(tests/objc/fwd_declarable_decl_in_same_file.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
