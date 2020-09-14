//===--- fwd_declarable_decl_in_same_file.m - test input file for iwyu ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Wno-objc-root-class

#import "fwd_declarable_decl_in_same_file.h"


@implementation Foo
@end

@implementation Bar
@end

/**** IWYU_SUMMARY

(tests/objc/fwd_declarable_decl_in_same_file.m has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
