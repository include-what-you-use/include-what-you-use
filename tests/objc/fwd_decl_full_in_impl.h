//===--- fwd_decl_full_in_impl.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

@class ImportedClass;
@protocol FooProtocol;

@interface C {
 @private
  ImportedClass <FooProtocol> *var;
}
@end

/**** IWYU_SUMMARY

(tests/objc/fwd_decl_full_in_impl.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
