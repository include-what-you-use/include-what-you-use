//===--- fwd_decl_class_as_method_argument.h - test input file for iwyu ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

@class ImportedClass;


@interface C {
}

- (void)handleFoo:(ImportedClass *)foo;
@end

/**** IWYU_SUMMARY

(tests/objc/fwd_decl_class_as_method_argument.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
