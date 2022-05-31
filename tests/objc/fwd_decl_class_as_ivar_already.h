//===--- fwd_decl_class_as_ivar_already.h - test input file for iwyu ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

@class ImportedClass;

@interface C {
 @private
  ImportedClass *var;
}
@end

/**** IWYU_SUMMARY

(tests/objc/fwd_decl_class_as_ivar_already.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
