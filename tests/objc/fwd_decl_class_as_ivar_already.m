//===--- fwd_decl_class_as_ivar_already.m - test input file for iwyu ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Wno-objc-root-class

#import "fwd_decl_class_as_ivar_already.h"

@implementation C
@end

/**** IWYU_SUMMARY

(tests/objc/fwd_decl_class_as_ivar_already.m has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
