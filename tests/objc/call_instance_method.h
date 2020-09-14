//===--- call_instance_method.h - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

@class Bar;

@interface Foo {
}
- (void)method:(Bar *)var;
@end

/**** IWYU_SUMMARY

(tests/objc/call_instance_method.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
